#include "fpdcommunity.h"
#include <QDebug>

FPDCommunity::FPDCommunity()
{

}

void FPDCommunity::Enroll(const QString &finger)
{
    qDebug() << "FPDCommunity::Enroll:" << finger;
    m_androidFP.enroll(100000); //nemo userID
}

void FPDCommunity::Identify()
{
    qDebug() << "FPDCommunity::Identify";
    m_androidFP.authenticate();
}
