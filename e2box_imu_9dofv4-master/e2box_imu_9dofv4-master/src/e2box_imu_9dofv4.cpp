#include "e2box_imu_9dofv4.h"

e2box_imu_9dofv4::e2box_imu_9dofv4()
{
    Initialize();
}

e2box_imu_9dofv4::~e2box_imu_9dofv4()
{
}

void e2box_imu_9dofv4::Initialize(void)
{//Initialize이거는 어디서 실행시켜 주는거지??
    m_dwordCounterChecksumPass = 0;
    m_dwordCounterChecksumFail = 0;

    memset(Euler,0,sizeof(Euler));
    memset(m_dAccel,0,sizeof(m_dAccel));
    memset(m_dAngRate,0,sizeof(m_dAngRate));
    //memset은 메모리를 특정 값으로 세팅한다.(memory + setting)
    //첫번째 인자는 메모리 시작 주소 + 두번째 인자 메모리에 세팅하고자 하는 값 집어넣음
    //세번째 인자는 길이*sizeof(데이터타입)
    //m_sPacket = "";

    SetStatusHeaderDetect(false);
    data_acquisition = false;
    SetStatusUpdateData(false);

    m_iRawDataIndex = 0;
}

void e2box_imu_9dofv4::ExtractData(unsigned char byteCurrent)
{
    if(GetStatusHeaderDetect() == false){
        if(byteCurrent == 0x2A){	// mark * 0x2A는 2진수로 변환하면 101010이다
            SetStatusHeaderDetect(true);
            memset(m_abyteRawData,0,sizeof(m_abyteRawData));
            m_abyteRawData[0] = byteCurrent;
            m_iRawDataIndex = 1;
        }
    }
    else{
        m_abyteRawData[m_iRawDataIndex++] = byteCurrent;
        if(byteCurrent == 0x0D){ // 0x0d = CR, 0x0A = LF
            for(int i = 0; i < m_iRawDataIndex; i++) m_acCopiedRawData[i] = m_abyteRawData[i];
            data_acquisition = true;
            SetStatusHeaderDetect(false);
        }
        if(m_iRawDataIndex > 90) SetStatusHeaderDetect(false);
    }
}

bool e2box_imu_9dofv4::HandlingDataIMU(){
    if(GetStatusHeaderDetect() == false){
        InterpretGeneral();
        SetStatusUpdateData(true);
        m_dwordCounterChecksumPass++;
    }
    else {
        m_dwordCounterChecksumFail++;
        return false;
    }
    return true;
}

// void e2box_imu_9dofv4::InterpretGeneral(void)
// {
//     char cBranch;
//     sscanf(&m_acCopiedRawData[0],"%c",&cBranch);
//     if(cBranch == '*'){
//         sscanf(&m_acCopiedRawData[1],"%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
//             &m_dQuaternion[0], &m_dQuaternion[1], &m_dQuaternion[2], &m_dQuaternion[3],
//             &m_dAngRate[0], &m_dAngRate[1], &m_dAngRate[2],
//             &m_dAccel[0], &m_dAccel[1], &m_dAccel[2]
//         );
//     }

//     SetStatusUpdateData(true);
// }
void e2box_imu_9dofv4::InterpretGeneral(void)
{
    char cBranch;
    sscanf(&m_acCopiedRawData[0],"%c",&cBranch);
    if(cBranch == '*'){
        sscanf(&m_acCopiedRawData[1],"%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
            &Euler[0], &Euler[1], &Euler[2],
            &m_dAngRate[0], &m_dAngRate[1], &m_dAngRate[2],
            &m_dAccel[0], &m_dAccel[1], &m_dAccel[2]
        );
    }

    SetStatusUpdateData(true);
}

BOOL e2box_imu_9dofv4::CalcCheckSum(void)
{
    sscanf(&m_acCopiedRawData[m_iRawDataIndex-3],"%x",&iChecksum);

    for(int i = 1; i < m_iRawDataIndex - 5; i++){
        if(i == 1) iCandisum = CheckBYTEXOR(m_acCopiedRawData[i], m_acCopiedRawData[i+1]);
        else iCandisum = CheckBYTEXOR(iCandisum, m_acCopiedRawData[i+1]);
    }

    if(iCandisum == iChecksum) return true;
    else return false;
}

DWORD e2box_imu_9dofv4::CheckBYTEXOR(BYTE byteCompared1, BYTE byteCompared2)
{
    BYTE byteMask = 0x80;
    BYTE byteResult = 0;

    for(int i = 0; i < 8; i++){
        byteResult <<= 1;

        if((byteCompared1 & byteMask) == (byteCompared2 & byteMask)) byteResult |= 0x0;
        else byteResult |= 0x1;

        byteMask >>= 1;
    }

    return byteResult;
}


