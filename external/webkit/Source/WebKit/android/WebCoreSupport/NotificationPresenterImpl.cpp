/*
 * Copyright 2007, The Android Open Source Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "NotificationPresenterImpl.h"

#if ENABLE(NOTIFICATIONS)

#include "KURL.h"
#include "Notification.h"
#include "ScriptExecutionContext.h"
#include "SecurityOrigin.h"
#include "WebViewCore.h"
#include "WebNotificationPermissionCallback.h"
#include "EventNames.h"
#include <wtf/HashMap.h>
#include <wtf/PassRefPtr.h>
#include "Logging.h"
#include <wtf/text/CString.h>
#include "SQLiteDatabase.h"
#include "SQLiteFileSystem.h"
#include "SQLiteStatement.h"
#include "SQLiteTransaction.h"

namespace android {

static const char* databaseName = "NotificationPermissions.db";

int NotificationPresenterImpl::s_counter;
String NotificationPresenterImpl::s_databasePath;
NotificationPresenterImpl::PermissionsMap NotificationPresenterImpl::s_notificationPermissions;
NotificationPresenterImpl::Notificationmap NotificationPresenterImpl::s_notificationMap;
NotificationPresenterImpl::NotificationIDmap NotificationPresenterImpl::s_notificationIDMap;
int NotificationPresenterImpl::mnotificationState;


NotificationPresenterImpl::NotificationPresenterImpl( WebViewCore *webViewCore)
  :m_webViewCore(webViewCore) ,m_result(1),m_callback(0) 
{
    ASSERT(m_webViewCore);
    maybeLoadPermanentPermissions();
}

bool NotificationPresenterImpl::show(PassRefPtr<Notification> notification)
{	
    WebCore::Notification* notify = notification.get();
    ScriptExecutionContext* sec = notify->scriptExecutionContext();
    String originStr = sec->securityOrigin()->toString();

    s_counter++;
    s_notificationMap.set(s_counter, notification );

    RefPtr<Notification> notificationptr = s_notificationMap.get(s_counter);

    int res = s_notificationMap.isEmpty();
    int size = s_notificationMap.size();

    KURL iconURL = notify->iconURL();
    String iconURLStr = iconURL.string();
    NotificationContents notcontent = notify->contents();
    String titleStr = notcontent.title();
    String bodyStr = notcontent.body();

    bool allow = s_notificationPermissions.get(originStr);
    if(allow || mnotificationState == NotificationPresenterImpl::AlwaysON ){
         m_webViewCore->notificationManagershow(iconURLStr, titleStr, bodyStr,s_counter); 
    }
    return true;    
}

void NotificationPresenterImpl::cancel(Notification* notification)
{
    int notificationID = s_notificationIDMap.get(notification);

    m_webViewCore->notificationManagerCancel(notificationID);

}

void NotificationPresenterImpl::notificationObjectDestroyed(Notification* notification)
{
    m_result = 1;
}

NotificationPresenter::Permission NotificationPresenterImpl::checkPermission(ScriptExecutionContext* context)
{
   String originStr = context->securityOrigin()->toString(); 

    bool isPresent = s_notificationPermissions.contains(originStr);

    if(isPresent){
        bool allow = s_notificationPermissions.get(originStr);
        if(allow)
             return NotificationPresenter::PermissionAllowed;
        else
             return NotificationPresenter::PermissionDenied;
    }
    else
       if(mnotificationState == NotificationPresenterImpl::AlwaysON)
             return NotificationPresenter::PermissionAllowed;
       else if(mnotificationState == NotificationPresenterImpl::Off)
             return NotificationPresenter::PermissionDenied;
       else
             return NotificationPresenter::PermissionNotAllowed; 
}

void NotificationPresenterImpl::requestPermission(ScriptExecutionContext* context, PassRefPtr<VoidCallback> callback)
{
    if(mnotificationState == NotificationPresenterImpl::OnDemand){
        m_callback = callback;
        String originStr = context->securityOrigin()->toString();

        bool isPresent = s_notificationPermissions.contains(originStr);
        if(!isPresent){   
            m_webViewCore->notificationPermissionsShowPrompt(originStr);
        }
        else{
            m_result = 0;
        }	
    }
}

void NotificationPresenterImpl::providePermissionState(const WTF::String& origin, bool allow)
{
    m_result = 0;
    s_notificationPermissions.set(origin,allow);
	if(m_callback)
        m_callback->handleEvent();
}

void NotificationPresenterImpl::cancelRequestsForPermission(WebCore::ScriptExecutionContext* context)
{
    m_webViewCore->notificationPermissionsHidePrompt();
    
}

void NotificationPresenterImpl::dispatchNotificationEvents(const WTF::String& eventName, int counter)
{
    int res = s_notificationMap.isEmpty();
    int size = s_notificationMap.size();
 
    RefPtr<Notification> notificationptr = s_notificationMap.get(counter);
 //   ScriptExecutionContext* sec = notificationptr->scriptExecutionContext();  prevent tool issue fix.

    if (notificationptr->scriptExecutionContext()){
        notificationptr->dispatchEvent(Event::create(eventName,false, true));   
    }

}

void NotificationPresenterImpl::recordNotificationID(int notificationID, int counter)
{
    RefPtr<Notification> notificationptr = s_notificationMap.get(counter); 
    s_notificationIDMap.set(notificationptr, notificationID);
}

void NotificationPresenterImpl::setSettingsValue(int notificationState)
{
    switch(notificationState)
    {
        case NotificationPresenterImpl::AlwaysON:
            mnotificationState = NotificationPresenterImpl::AlwaysON;
        break;
        case NotificationPresenterImpl::OnDemand:
            mnotificationState = NotificationPresenterImpl::OnDemand;
        break;
        case NotificationPresenterImpl::Off:
            mnotificationState = NotificationPresenterImpl::Off;
        break;

    } 
}


//Database Related
void NotificationPresenterImpl::setDatabasePath(String path)
{
    // Take the first non-empty value.
    if (s_databasePath.length() > 0)
        return;
    s_databasePath = path;
}

bool NotificationPresenterImpl::openDatabase(SQLiteDatabase* database)
{    
    ASSERT(database);
    String filename = SQLiteFileSystem::appendDatabaseFileNameToPath(s_databasePath, databaseName);
    if (!database->open(filename))
        return false;
    if (chmod(filename.utf8().data(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) {
        database->close();
        return false;
    }
    return true;
}

void NotificationPresenterImpl::maybeLoadPermanentPermissions()
{
    
    SQLiteDatabase database;
    if (!openDatabase(&database))
        return;

    // Create the table here, such that even if we've just created the DB, the
    // commands below should succeed.
    if (!database.executeCommand("CREATE TABLE IF NOT EXISTS NotifyPermissions (origin TEXT UNIQUE NOT NULL, allow INTEGER NOT NULL)")) {
	database.close();
        return;
    }

    SQLiteStatement statement(database, "SELECT * FROM NotifyPermissions");
    if (statement.prepare() != SQLResultOk) {
        database.close();
        return;
    }

    ASSERT(s_notificationPermissions.size() == 0);
    while (statement.step() == SQLResultRow)
        s_notificationPermissions.set(statement.getColumnText(0), statement.getColumnInt64(1));

    database.close();
}

void NotificationPresenterImpl::maybeStorePermanentPermissions()
{
    // If the permanent permissions haven't been modified, there's no need to
    // save them to the DB. (If we haven't even loaded them, writing them now
    // would overwrite the stored permissions with the empty set.)
    

    SQLiteDatabase database;
    if (!openDatabase(&database))
        return;

    SQLiteTransaction transaction(database);

    // The number of entries should be small enough that it's not worth trying
    // to perform a diff. Simply clear the table and repopulate it.
    if (!database.executeCommand("DELETE FROM NotifyPermissions")) {
        database.close();
        return;
    }

    PermissionsMap::const_iterator end = s_notificationPermissions.end();
    for (PermissionsMap::const_iterator iter = s_notificationPermissions.begin(); iter != end; ++iter) {
         SQLiteStatement statement(database, "INSERT INTO NotifyPermissions (origin, allow) VALUES (?, ?)");
         if (statement.prepare() != SQLResultOk)
             continue;
         statement.bindText(1, iter->first);
         statement.bindInt64(2, iter->second);
         statement.executeCommand();
    }

    transaction.commit();
    database.close();

}

void NotificationPresenterImpl::deleteDatabase()
{
    SQLiteDatabase database;
    if (!openDatabase(&database))
        return;

    SQLiteTransaction transaction(database);

    if (!database.executeCommand("DELETE FROM NotifyPermissions")) {
        database.close();
        return;
    }

    transaction.commit();
    database.close();
}

void NotificationPresenterImpl::clearAll()
{
    deleteDatabase();
    s_notificationPermissions.clear();
}

} // namespace android

#endif // ENABLE(NOTIFICATIONS)
