// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Hardware.FilterWheel.ZWO.EFWheel.h"

#include "EFW_filter.h"

#include <QDebug>

class EFWException : public std::exception {
public:
    EFWException( EFW_ERROR_CODE _errorCode ) :
        errorCode( _errorCode )
    {
    }

    const char* what() const noexcept override
    {
        switch( errorCode ) {
            case EFW_ERROR_INVALID_INDEX: return "Invalid index";
            case EFW_ERROR_INVALID_ID: return "Invalid id";
            case EFW_ERROR_INVALID_VALUE: return "Invalid value";
            case EFW_ERROR_REMOVED: return "Filter wheel removed";
            case EFW_ERROR_MOVING: return "Filter wheel moving";
            case EFW_ERROR_ERROR_STATE: return "Filter wheel error state";
            case EFW_ERROR_GENERAL_ERROR: return "Filter wheel general error";
            case EFW_ERROR_NOT_SUPPORTED: return "Filter wheel operation not supported";
            case EFW_ERROR_CLOSED: return "Filter wheel closed";
            default:
                return 0;
        }
    }

private:
    EFW_ERROR_CODE errorCode;
};

static void checkResult( EFW_ERROR_CODE errorCode )
{
    if( errorCode != EFW_SUCCESS ) {
        throw EFWException( errorCode );
    }
}
std::shared_ptr<EFWheel> EFWheel::Open()
{
    auto result = std::make_shared<EFWheel>();
    if( result->open() ) {
        return result;
    }
    return 0;
}

bool EFWheel::open()
{
    int count = EFWGetNum();
    if( count > 0 ) {
        checkResult( EFWGetID( 0, &id ) );
        checkResult( EFWOpen( id ) );
        EFW_INFO info;
        checkResult( EFWGetProperty( id, &info ) );
        slotsCount = info.slotNum;
        return true;
    }
    return false;
}

int EFWheel::GetPosition()
{
    if( slotsCount > 0 ) {
        int pos = -1;
        checkResult( EFWGetPosition( id, &pos ) );
        return pos;
    }
    return -1;
}

void EFWheel::SetPosition( int pos )
{
    if( pos < slotsCount ) {
        checkResult( EFWSetPosition( id, pos ) );
    }
}

void EFWheel::Close()
{
    if( slotsCount > 0 ) {
        checkResult( EFWClose( id ) );
        slotsCount = 0;
    }
}
