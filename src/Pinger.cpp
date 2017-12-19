#include "Pinger.h"


CPinger::CPinger()
{
    m_bNetworkInitialized = 0;
    m_bTargetSet = 0;
    m_iSeqCounter = 0;
    m_IcmpRawSocket = 0;
    m_IcmpHeader.i_type = 8;
    m_IcmpHeader.i_code = 0;
    m_IcmpHeader.i_id = GetCurrentProcessId();
    m_IcmpHeader.i_cksum = 0;
    m_IcmpHeader.i_seq = 0;
    cReplies = 0;
    cTimeOuts = 0;
    cTotal = 0;
}

CPinger::~CPinger()
{
    if( m_bNetworkInitialized )
        DoneNetwork();
}

BOOL CPinger::InitNetwork()
{
    struct WSAData _WSAData;
    if( ! m_bNetworkInitialized )
    {
        if( WSAStartup( WINSOCK_VERSION, &_WSAData ) )
            return 0;
        m_IcmpRawSocket = socket( AF_INET, SOCK_RAW, IPPROTO_ICMP );
        if( m_IcmpRawSocket == static_cast<SOCKET>( INVALID_SOCKET ) )
            return 0;
        u_long argp = 1;
        if( ioctlsocket( m_IcmpRawSocket, FIONBIO, &argp ) == SOCKET_ERROR )
            return 0;
        m_bNetworkInitialized = 1;
    }
    return 1;
}

BOOL CPinger::DoneNetwork()
{
    if( m_bNetworkInitialized )
    {
        if( closesocket( m_IcmpRawSocket ) == SOCKET_ERROR )
        {
            return 0;
        }
        else return WSACleanup() != SOCKET_ERROR;
    }
    return 1;
}

BOOL CPinger::SetTargetName( LPCSTR lpcszTargetName )
{
    if( lpcszTargetName && m_bNetworkInitialized )
    {
        memset( & m_TargetAddr, 0, sizeof( m_TargetAddr ) );
        m_TargetAddr.sin_family = AF_INET;
        if( isdigit( * lpcszTargetName ) )
        {
            m_TargetAddr.sin_addr.S_un.S_addr = inet_addr( lpcszTargetName );
            if( m_TargetAddr.sin_addr.S_un.S_addr != INADDR_NONE )
            {
                m_bTargetSet = 1;
                return 1;
            }
        }
        else
        {
            struct hostent * remoteHost = gethostbyname( lpcszTargetName );
            if( remoteHost != NULL )
            {
                memcpy( & m_TargetAddr.sin_addr, remoteHost->h_addr_list[ 0 ], remoteHost->h_length );
                m_bTargetSet = 1;
                return 1;
            }
        }
    }
    return 0;
}

BOOL CPinger::GetStatistic( BYTE * lpcReplies, BYTE * lpcTotal )
{
    if( lpcReplies )
        * lpcReplies = cReplies;
    if( lpcTotal )
        * lpcTotal = cTotal;
    return 1;
}

BOOL CPinger::SendEcho()
{
    static_assert( sizeof( IcmpHeader ) == 8, "Wrong size of IcmpHeader." );
    static_assert( sizeof( sockaddr_in ) == 16, "Wrong size of sockaddr_in." );
    m_iSeqCounter++;
    m_IcmpHeader.i_cksum = 0;
    m_IcmpHeader.i_seq = static_cast<uint16_t>( m_iSeqCounter );
    m_IcmpHeader.i_cksum = checksum( reinterpret_cast<uint16_t * >( & m_IcmpHeader ), sizeof( m_IcmpHeader ) );
    if( sendto( m_IcmpRawSocket,
                reinterpret_cast< const char * >( & m_IcmpHeader ), sizeof( m_IcmpHeader ), 0,
                reinterpret_cast< const struct sockaddr * >( & m_TargetAddr ), sizeof( m_TargetAddr ) ) == SOCKET_ERROR )
    {
        return 0;
    }
    else
    {
        m_dwLastSendTime = GetTickCount();
        cTotal++;
        return 1;
    }
}

BOOL CPinger::RecvEcho()
{
    u_long argp;
    ioctlsocket( m_IcmpRawSocket, FIONREAD, & argp );
    if( argp )
    {
        const int BufLen = 256;
        char RecvBuf[ BufLen ];
        struct sockaddr from;
        int fromlen = sizeof( from );
        if( recvfrom( m_IcmpRawSocket, RecvBuf, BufLen, 0, & from, & fromlen ) == SOCKET_ERROR )
        {
            return 0;
        }
        else if( * reinterpret_cast<uint32_t * >( & from.sa_data[ 2 ] ) == m_TargetAddr.sin_addr.S_un.S_addr )
        {
            cReplies++;
            return 1;
        }
        else return 0;
    }
    else return 0;
}

uint16_t CPinger::checksum( uint16_t * Buf, size_t SizeInBytes )
{
    size_t Sum = 0;
    size_t SizeRemain = SizeInBytes;
    if( SizeInBytes > 1 )
    {
        size_t Count = SizeInBytes >> 1;
        SizeRemain -= 2 * Count;
        do
        {
            Sum += * Buf;
            Buf++;
            Count--;
        }
        while( Count );
    }
    if( SizeRemain )    // SizeInBytes % 2 != 0
        Sum += * reinterpret_cast< uint8_t * >( Buf );
    unsigned int TmpSum = uint16_t( Sum ) + ( Sum >> 16 );
    return ~( TmpSum + ( TmpSum >> 16 ) );
}
