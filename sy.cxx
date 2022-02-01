//
// sy: a Windows command-line app that prepares removable drives for removal
// The logic: Flush out buffers first in case lock fails and removal has to happen regardless.
//            Lock the volume so no other apps can't write to it.
//            Flush again in case any other app writes snuck in before the lock.
//            Dismount the volume so it can be unplugged.
//

#include <windows.h>
#include <stdio.h>

bool DismountVolume( HANDLE h )
{
    DWORD dwRet = 0;

    if ( DeviceIoControl( h, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &dwRet, NULL ) )
    {
        printf( " dismounted" );
        return TRUE;
    }

    printf( " FSCTL_DISMOUNT_VOLUME failed, error %d\n", GetLastError() );
    return false;
} //DismountVolume

bool LockVolume( HANDLE h )
{
    DWORD dwRet = 0;

    if ( DeviceIoControl( h, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &dwRet, NULL ) )
    {
        printf( " locked..." );
        return TRUE;
    }

    printf( " FSCTL_LOCK_VOLUME failed, error %d\n", GetLastError() );
    return false;
} //LockVolume

bool FlushBuffers( HANDLE h )
{
    if ( FlushFileBuffers( h ) )
    {
        printf( " flushed..." );
        return true;
    }

    printf( " FlushFileBuffers failed, error %d \n", GetLastError() ) ;
    return false;
} //FlushBuffers

bool SyncDrive( WCHAR c )
{
    WCHAR file[20];
    wcscpy_s( file, _countof( file), L"\\\\.\\c:" );
    file[ 4 ] = c;

    printf( " flushing, locking, flushing (again), then dismounting %wc:  \n", c );
    
    HANDLE h = CreateFile( file, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    
    if ( INVALID_HANDLE_VALUE == h )
    {
        printf( "can't open file %ws, error %d\n", file, GetLastError() );
        return false;
    }

    bool ok = FlushBuffers( h );

    if ( ok )
        ok = LockVolume( h );

    if ( ok )
        ok = FlushBuffers( h );

    if ( ok )
        ok = DismountVolume( h );

    if ( ok )
        printf( "\n volume %wc: successful sync\n", c );

    CloseHandle( h );

    return ok;
} //SyncDrive

#if false

void SyncDrives( char cDrive )
{
    if ( 0 != cDrive )
    {
        SyncDrive( cDrive );
    }
    else
    {
        DWORD dwDriveMask = GetLogicalDrives();
    
        for ( WCHAR c = L'a'; c <= L'z'; c++ )
        {
            DWORD dwTemp = dwDriveMask & 1;
            dwDriveMask >>= 1;
    
            if ( 0 != dwTemp )
            {
                WCHAR rootPath[ 10 ];
                wcscpy_s( rootPath, _countof( rootPath ), L"?:\\" );
                rootPath[0] = c;
                DWORD dwRC = GetDriveType( rootPath );
          
                if ( DRIVE_REMOTE != dwRC )
                    SyncDrive( c );
            }
        }
    }
} //SyncDrives

#endif

void Usage()
{
    printf( "usage:  sy [X]\n" );
    printf( "  Use sy.exe before unplugging an external drive\n" );
    printf( "  arguments:  [X] -- drive letter to flush, lock, flush (again), then dismount.\n" );
    printf( "  examples:   sy r\n" );
    printf( "              sy f:\n" );

    exit( EXIT_FAILURE );
} //Usage

extern "C" int __cdecl wmain( int argc, WCHAR * argv[] )
{
    WCHAR cDrive = 0;

    if ( 2 == argc )
        cDrive = argv[1][0];
    else 
        Usage();

    bool ok = SyncDrive( cDrive );

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
} //main

