#include <qt/mintingtablemodel.h>
#include <qt/mintingfilterproxy.h>

#include <kernelrecord.h>
#include <key_io.h>
#include <interfaces/wallet.h>
#include <qt/transactiondesc.h>
#include <qt/transactionrecord.h>
//#include <qt/guiutil.h>
#include <qt/walletmodel.h>
#include <qt/guiconstants.h>
#include <qt/bitcoinunits.h>
#include <qt/optionsmodel.h>
#include <qt/addresstablemodel.h>

#include <wallet/wallet.h>
#include <validation.h>
#include <chainparams.h>
#include <timedata.h>

#include <QColor>
#include <QTimer>

// Amount column is right-aligned it contains numbers
static int column_alignments[] = {
        Qt::AlignLeft|Qt::AlignVCenter,
        Qt::AlignLeft|Qt::AlignVCenter,
        Qt::AlignRight|Qt::AlignVCenter,
        Qt::AlignRight|Qt::AlignVCenter,
        Qt::AlignRight|Qt::AlignVCenter,
        Qt::AlignRight|Qt::AlignVCenter
    };

// Private implementation
class MintingTablePriv
{
public:
    MintingTablePriv(WalletModel *walletModel, MintingTableModel *parent):
            walletModel(walletModel),
            parent(parent)
    {
    }
    WalletModel *walletModel;
    MintingTableModel *parent;

    /* Local cache of wallet.
     * As it is in the same order as the CWallet, by definition
     * this is sorted by sha256.
     */
    QList<KernelRecord> cachedWallet;

    /* Query the wallet's unspent outputs anew from core: one call, one
     * wallet lock. (The port walked every wallet transaction and made four
     * locked calls per output, in the GUI thread, which froze the window for
     * a long time on wallets with many stakes. It then kept the list up to
     * date with a binary search by hash, but the list was never sorted -
     * mapWallet is unordered - so rows went missing or were duplicated.)
     */
    void refreshWallet()
    {
        cachedWallet.clear();
        const std::vector<interfaces::WalletMintingOutput> outputs = walletModel->wallet().getMintingOutputs();
        cachedWallet.reserve(outputs.size());
        for (const auto& out : outputs) {
            CTxDestination address;
            std::string addrStr = ExtractDestination(out.txout.scriptPubKey, address) ? EncodeDestination(address) : out.to;
            cachedWallet.append(KernelRecord(out.hash, out.time, addrStr, out.txout.nValue, out.n, false));
        }
    }

    int size()
    {
        return cachedWallet.size();
    }

    KernelRecord *index(int idx)
    {
        if(idx >= 0 && idx < cachedWallet.size())
        {
            KernelRecord *rec = &cachedWallet[idx];
            return rec;
        }
        else
        {
            return 0;
        }
    }

    QString describe(TransactionRecord *rec)
    {
        {
            return TransactionDesc::toHTML(walletModel->node(), walletModel->wallet(), rec, BitcoinUnits::Unit::BTC);  
        }
        return QString("");
    }

};

static void NotifyTransactionChanged(MintingTableModel *ttm, const uint256 &hash, ChangeType status)
{
    QMetaObject::invokeMethod(ttm, "updateTransaction", Qt::QueuedConnection,
                              Q_ARG(QString, QString::fromStdString(hash.GetHex())),
                              Q_ARG(int, status));
}

MintingTableModel::MintingTableModel(WalletModel *parent) :
        QAbstractTableModel(parent),
        walletModel(parent),
        mintingInterval(1440*30),
        priv(new MintingTablePriv(walletModel, this)),
        cachedNumBlocks(0)
{
    columns << tr("Transaction") <<  tr("Address") << tr("Age") << tr("Balance") << tr("CoinDay") << tr("MintProbability");

    updateDifficulty();
    priv->refreshWallet();

    // Changes are gathered and the list is reloaded at most once a second,
    // so a rescan or a burst of stakes doesn't redo it for every transaction
    m_refresh_timer = new QTimer(this);
    m_refresh_timer->setSingleShot(true);
    m_refresh_timer->setInterval(1000);
    connect(m_refresh_timer, &QTimer::timeout, this, &MintingTableModel::refreshNow);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateAge()));
    // (ages are in days; the port's MODEL_UPDATE_DELAY*1000 came out as
    // 1000 seconds after MODEL_UPDATE_DELAY became a duration)
    timer->start(60 * 1000);

    // (the port used the old Qt signal name with an int, which no longer
    // exists, so the minting list never followed unit changes)
    connect(walletModel->getOptionsModel(), &OptionsModel::displayUnitChanged, this, &MintingTableModel::updateDisplayUnit);
    m_handler_transaction_changed = walletModel->wallet().handleTransactionChanged(std::bind(NotifyTransactionChanged, this, std::placeholders::_1, std::placeholders::_2));
}

MintingTableModel::~MintingTableModel()
{
    m_handler_transaction_changed->disconnect();
    delete priv;
}

void MintingTableModel::updateTransaction(const QString &hash, int status)
{
    Q_UNUSED(hash);
    Q_UNUSED(status);
    if (!m_refresh_timer->isActive()) m_refresh_timer->start();
}

void MintingTableModel::refreshNow()
{
    beginResetModel();
    updateDifficulty();
    priv->refreshWallet();
    endResetModel();
    if (mintingProxyModel) mintingProxyModel->invalidate();
}

void MintingTableModel::updateDifficulty()
{
    // Taken once per refresh rather than once per row (and per sort
    // comparison when the list is sorted by probability)
    const CBlockIndex* tip = walletModel->getTip();
    const CBlockIndex* p = tip ? GetLastBlockIndex(tip, true) : nullptr;
    m_difficulty = p ? p->GetBlockDifficulty() : 0.0;
}

void MintingTableModel::updateAge()
{
    updateDifficulty();
    Q_EMIT dataChanged(index(0, Age), index(priv->size()-1, Age));
    Q_EMIT dataChanged(index(0, CoinDay), index(priv->size()-1, CoinDay));
    Q_EMIT dataChanged(index(0, MintProbability), index(priv->size()-1, MintProbability));
}

void MintingTableModel::setMintingProxyModel(MintingFilterProxy *mintingProxy)
{
    mintingProxyModel = mintingProxy;
}

int MintingTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return priv->size();
}

int MintingTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return columns.length();
}

QVariant MintingTableModel::data(const QModelIndex &index, int role) const
{
    const Consensus::Params& params = Params().GetConsensus();
    if(!index.isValid())
        return QVariant();
    KernelRecord *rec = static_cast<KernelRecord*>(index.internalPointer());

    switch(role)
    {
      case Qt::DisplayRole:
        switch(index.column())
        {
        case Address:
            return formatTxAddress(rec, false);
        case TxHash:
            return formatTxHash(rec);
        case Age:
            return formatTxAge(rec);
        case Balance:
            return formatTxBalance(rec);
        case CoinDay:
            return formatTxCoinDay(rec);
        case MintProbability:
            return formatDayToMint(rec);
        }
        break;
      case Qt::TextAlignmentRole:
        return column_alignments[index.column()];
        break;
      case Qt::ToolTipRole:
        switch(index.column())
        {
        case MintProbability:
            int interval = this->mintingInterval;
            QString unit = tr("minutes");

            int hours = interval / 60;
            int days = hours  / 24;

            if(hours > 1) {
                interval = hours;
                unit = tr("hours");
            }
            if(days > 1) {
                interval = days;
                unit = tr("days");
            }

            QString str = QString(tr("You have %1 chance to find a POS block if you mint %2 %3 at current difficulty."));
            return str.arg(index.data().toString().toUtf8().constData()).arg(interval).arg(unit);
        }
        break;
      case Qt::EditRole:
        switch(index.column())
        {
        case Address:
            return formatTxAddress(rec, false);
        case TxHash:
            return formatTxHash(rec);
        case Age:
            return qint64(rec->getAge());
        case CoinDay:
            return qint64(rec->getCoinAge());
        case Balance:
            return qint64(rec->nValue);
        case MintProbability:
            return getDayToMint(rec);
        }
        break;
      case Qt::BackgroundColorRole:
        // LiteDoge PoS v2: a coin can stake once it is nStakeMinAge (8 hours)
        // old, and its chance doesn't grow after that (the port compared
        // whole days, so 8 hours rounded to 0 and every coin looked mature)
        const int64_t ageSecs = TicksSinceEpoch<std::chrono::seconds>(GetAdjustedTime()) - rec->nTime;
        if (ageSecs < params.nStakeMinAge)
        {
            return COLOR_MINT_YOUNG;
        }
        else
        {
            return COLOR_MINT_OLD;
        }
        break;

    }
    return QVariant();
}

void MintingTableModel::setMintingInterval(int interval)
{
    mintingInterval = interval;
}

QString MintingTableModel::lookupAddress(const std::string &address, bool tooltip) const
{
    QString label = walletModel->getAddressTableModel()->labelForAddress(QString::fromStdString(address));
    QString description;
    if(!label.isEmpty())
    {
        description += label + QString(" ");
    }
    if(label.isEmpty() || tooltip)
    {
        description += QString(" (") + QString::fromStdString(address) + QString(")");
    }
    return description;
}

double MintingTableModel::getDayToMint(KernelRecord *wtx) const
{
    double prob = wtx->getProbToMintWithinNMinutes(m_difficulty, mintingInterval);
    prob = prob * 100;
    return prob;
}

QString MintingTableModel::formatDayToMint(KernelRecord *wtx) const
{
    double prob = getDayToMint(wtx);
    return QString::number(prob, 'f', 6) + "%";
}

QString MintingTableModel::formatTxAddress(const KernelRecord *wtx, bool tooltip) const
{
    return QString::fromStdString(wtx->address);
}

QString MintingTableModel::formatTxHash(const KernelRecord *wtx) const
{
    return QString::fromStdString(wtx->hash.ToString());
}

QString MintingTableModel::formatTxCoinDay(const KernelRecord *wtx) const
{
    return QString::number(wtx->getCoinAge());
}

QString MintingTableModel::formatTxAge(const KernelRecord *wtx) const
{
    int64_t nAge = wtx->getAge();
    return QString::number(nAge);
}

QString MintingTableModel::formatTxBalance(const KernelRecord *wtx) const
{
    return BitcoinUnits::format(walletModel->getOptionsModel()->getDisplayUnit(), wtx->nValue);
}

QVariant MintingTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal)
    {
        if(role == Qt::DisplayRole)
        {
            return columns[section];
        }
        else if (role == Qt::TextAlignmentRole)
        {
            return column_alignments[section];
        } else if (role == Qt::ToolTipRole)
        {
            switch(section)
            {
            case Address:
                return tr("Destination address of the output.");
            case TxHash:
                return tr("Original transaction id.");
            case Age:
                return tr("Age of the transaction in days.");
            case Balance:
                return tr("Balance of the output.");
            case CoinDay:
                return tr("Coin age in the output.");
            case MintProbability:
                return tr("Chance to mint a block within given time interval.");
            }
        }
    }
    return QVariant();
}

QModelIndex MintingTableModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    KernelRecord *data = priv->index(row);
    if(data)
    {
        return createIndex(row, column, priv->index(row));
    }
    else
    {
        return QModelIndex();
    }
}

void MintingTableModel::updateDisplayUnit()
{
    // emit dataChanged to update Balance column with the current unit
    Q_EMIT dataChanged(index(0, Balance), index(priv->size()-1, Balance));
}
