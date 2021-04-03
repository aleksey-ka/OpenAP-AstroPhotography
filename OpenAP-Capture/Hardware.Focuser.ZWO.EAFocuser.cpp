// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Hardware.Focuser.ZWO.EAFocuser.h"

#include "EAF_focuser.h"

#include <QThread>
#include <QDebug>

class EAFException : public std::exception {
public:
    EAFException( EAF_ERROR_CODE _errorCode ) :
        errorCode( _errorCode )
    {
    }

    const char* what() const noexcept override
    {
        switch( errorCode ) {
            case EAF_ERROR_INVALID_INDEX: return "Invalid index";
            case EAF_ERROR_INVALID_ID: return "Invalid id";
            case EAF_ERROR_INVALID_VALUE: return "Invalid value";
            case EAF_ERROR_REMOVED: return "EAF wheel removed";
            case EAF_ERROR_MOVING: return "EAF moving";
            case EAF_ERROR_ERROR_STATE: return "EAF error state";
            case EAF_ERROR_GENERAL_ERROR: return "EAF general error";
            case EAF_ERROR_NOT_SUPPORTED: return "EAF operation not supported";
            case EAF_ERROR_CLOSED: return "EAF closed";
            default:
                return 0;
        }
    }

private:
    EAF_ERROR_CODE errorCode;
};

static void checkResult( EAF_ERROR_CODE errorCode )
{
    if( errorCode != EAF_SUCCESS ) {
        throw EAFException( errorCode );
    }
}

std::shared_ptr<Focuser> ZWOFocuser::Open()
{
    if( EAFGetNum()  > 0 ) {
        int id = -1;
        checkResult( EAFGetID( 0, &id ) );
        auto focuser = std::make_shared<ZWOFocuser>();
        if( focuser->connect( id ) ) {
            return focuser;
        }
    }
    return 0;
}

bool ZWOFocuser::connect( int _id )
{
    id = _id;
    if( EAFOpen( id ) == EAF_SUCCESS ) {
        EAFGetPosition( id, &focuserPos );
        return true;
    }
    return false;
}

void ZWOFocuser::Close()
{
    if( id >= 0 ) {
        cancelMoveTo();
        checkResult( EAFClose( id ) );
        focuserPos = INT_MIN;
        id = -1;
    }
}

void ZWOFocuser::Forward()
{
    if( isInsideMoveTo() ) {
        cancelMoveTo();
        return;
    }
    syncMoveTo( focuserPos + stepsToGo );
}

void ZWOFocuser::Backward()
{
    if( isInsideMoveTo() ) {
        cancelMoveTo();
        return;
    }
    syncMoveTo( focuserPos - stepsToGo );
}

void ZWOFocuser::MarkZero()
{
    cancelMoveTo();
    checkResult( EAFResetPostion( id, 0 ) );
}

void ZWOFocuser::GoToPos( int pos )
{
   focuserPos = INT_MIN;
   checkResult( EAFMove( id, pos ) );
   targetPos = pos;
}

void ZWOFocuser::syncMoveTo( int newPos )
{
    focuserPos = INT_MIN;
    checkResult( EAFMove( id, newPos ) );
    while( isInsideMoveTo() ) {
        QThread::msleep( 100 );
    }
    checkResult( EAFGetPosition( id, &focuserPos ) );
}

bool ZWOFocuser::isInsideMoveTo() const
{
    if( focuserPos == INT_MIN && targetPos != INT_MIN ) {
        return true;
    }
    bool isMoving = false;
    checkResult( EAFIsMoving( id, &isMoving, 0 ) );
    return isMoving;
}

void ZWOFocuser::cancelMoveTo() const
{
    EAFStop( id );
    EAFGetPosition( id, &focuserPos );
}
