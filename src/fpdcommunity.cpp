#include "fpdcommunity.h"
#include <QDebug>

FPDCommunity::FPDCommunity()
{
    qDebug() << Q_FUNC_INFO;
}

void FPDCommunity::Enroll(const QString &finger)
{
    qDebug() << Q_FUNC_INFO << finger;
    m_androidFP.enroll(100000); //nemo userID
}

void FPDCommunity::Identify()
{
    qDebug() << Q_FUNC_INFO;
    m_androidFP.authenticate();
}
