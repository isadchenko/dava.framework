/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "AssetCache/Test/AssetCacheTest.h"
#include "AssetCache/AssetCache.h"

#include "FileSystem/FileList.h"


#include "Network/Base/Endpoint.h"



namespace DAVA
{
    
namespace AssetCache
{
    
class ClientTest: public DAVA::AssetCache::ClientDelegate
{
public:
    
    DAVA::AssetCache::Client *client = nullptr;
    DAVA::AssetCache::ClientCacheEntry clientEntry;
    
    bool testIsRunning = false;
    
public:
    
    ClientTest(DAVA::AssetCache::Client *_client)
    : client(_client)
    {
        client->SetDelegate(this);
    }
    
    void OnAddedToCache(const DAVA::AssetCache::CacheItemKey &key, bool added) override
    {
        Logger::FrameworkDebug("=====    ClientTest::%s start    =====", __FUNCTION__);
        Logger::FrameworkDebug("\tadded is %d", added);
        testIsRunning = false;
        Logger::FrameworkDebug("=====    ClientTest::%s finish   =====", __FUNCTION__);
    }
    
    void OnIsInCache(const DAVA::AssetCache::CacheItemKey &key, bool isInCache) override
    {
        Logger::FrameworkDebug("=====    ClientTest::%s start    =====", __FUNCTION__);
        
        Logger::FrameworkDebug("\tisInCache is %d", isInCache);
        
        if(isInCache)
        {
            auto getRequestSent = client->GetFromCache(key);
            Logger::FrameworkDebug("\tgetRequestSent is %d", getRequestSent);
        }
        else
        {
            auto addRequestSent = client->AddToCache(key, clientEntry.GetFiles());
            Logger::FrameworkDebug("\taddRequestSent is %d", addRequestSent);
        }
        
        Logger::FrameworkDebug("=====    ClientTest::%s finish   =====", __FUNCTION__);
    }
    
    void OnReceivedFromCache(const DAVA::AssetCache::CacheItemKey &key, const DAVA::AssetCache::CachedFiles &files) override
    {
        Logger::FrameworkDebug("=====    ClientTest::%s start    =====", __FUNCTION__);
        
        bool entriesAreEqual = (key == clientEntry.GetKey());
        Logger::FrameworkDebug("\tentriesAreEqual is %d", entriesAreEqual);
        
        auto filesSet = files.GetFiles();
        auto count = files.GetFiles().size();
        
        Logger::FrameworkDebug("....Dump of files....");
        Logger::FrameworkDebug("\tfiles count is %d", count);
        for(auto & f: filesSet)
        {
            Logger::FrameworkDebug("\t\t %s", f.GetStringValue().c_str());
        }
        Logger::FrameworkDebug("........ Done ........");
        
        testIsRunning = false;
        Logger::FrameworkDebug("=====    ClientTest::%s finish   =====", __FUNCTION__);
    }
    
    
    void Run(const DAVA::AssetCache::ClientCacheEntry &entry)
    {
        clientEntry = entry;
        
        testIsRunning = true;
        client->IsInCache(clientEntry.GetKey());
        
        while(testIsRunning)
        {
            //wait
        }
    }
};

class ServerTest: public DAVA::AssetCache::ServerDelegate
{
public:
    
    DAVA::AssetCache::Server *server = nullptr;
    DAVA::AssetCache::CacheDB dataBase;
    
public:
    
    ServerTest(DAVA::AssetCache::Server *_server)
    :   server(_server)
    ,   dataBase("/Users/victorkleschenko/Downloads/__AssetCacheTest/CacheFolder", 1 * 1024 * 1024)
    {
        server->SetDelegate(this);
    }
    
    void OnAddedToCache(const DAVA::AssetCache::CacheItemKey &key, const DAVA::AssetCache::CachedFiles &files) override
    {
        Logger::FrameworkDebug("=====    ServerTest::%s start    =====", __FUNCTION__);
        if(server)
        {
            DVASSERT(false);
//            dataBase.Insert(key, files);
            
            server->FilesAddedToCache(key, true);
        }
        Logger::FrameworkDebug("=====    ServerTest::%s finish   =====", __FUNCTION__);
    }
    
    void OnIsInCache(const DAVA::AssetCache::CacheItemKey &key) override
    {
        Logger::FrameworkDebug("=====    ServerTest::%s start    =====", __FUNCTION__);
        if(server)
        {
            bool inCache = dataBase.Contains(key);
            server->FilesInCache(key, inCache);
        }
        Logger::FrameworkDebug("=====    ServerTest::%s finish   =====", __FUNCTION__);
    }
    
    void OnRequestedFromCache(const DAVA::AssetCache::CacheItemKey &key) override
    {
        Logger::FrameworkDebug("=====    ServerTest::%s start    =====", __FUNCTION__);
        
        if(server)
        {
            bool inCache = dataBase.Contains(key);
            if(inCache)
            {
                DVASSERT(false);
//                server->SendFiles(key, cachedFiles);
            }
            else
            {
                server->SendFiles(key, DAVA::AssetCache::CachedFiles());
            }
        }
        
        Logger::FrameworkDebug("=====    ServerTest::%s finish   =====", __FUNCTION__);
    }
};
    
void RunPackerTest()
{
// --- INITIALIZATION OF NETWORK ---
    static const DAVA::uint16 port = 5566;
    
    auto server = new DAVA::AssetCache::Server();
    server->Listen(port);
    
    auto client = new DAVA::AssetCache::Client();
    client->Connect("127.0.0.1", port);
    
    Net::NetCore::Instance()->RestartAllControllers();
    while(client->IsConnected() == false || server->IsConnected() == false)
    {
        sleep(10);
    }
// --- INITIALIZATION OF NETWORK ---
    
    
// --- INITIALIZATION OF FOLDER TEST ---
    const FilePath inputFolder("/Users/victorkleschenko/Downloads/__AssetCacheTest/TestProject/InFolder/");
    DAVA::AssetCache::ClientCacheEntry cacheEntry;
    cacheEntry.AddParam("Resource Packer 1.0");
    cacheEntry.AddParam("RGBA8888");
    cacheEntry.AddParam("split");

    const FilePath processFolder("/Users/victorkleschenko/Downloads/__AssetCacheTest/TestProject/$process/");
    ScopedPtr<FileList> flist(new FileList(processFolder));
    auto count = flist->GetCount();
    for(auto i = 0; i < count; ++i)
    {
        if(!flist->IsDirectory(i))
        {
            cacheEntry.AddFile(flist->GetPathname(i));
        }
    }
    
    uint8 digest[MD5::DIGEST_SIZE];
    MD5::ForDirectory(inputFolder, digest, false, false);
    cacheEntry.InvalidatePrimaryKey(digest);
    cacheEntry.InvalidateSecondaryKey();
// --- INITIALIZATION OF FOLDER TEST ---
    
    // --- RUNNING TEST ---
    ServerTest serverTest(server);
    ClientTest clientTest(client);
    
    clientTest.Run(cacheEntry);
    clientTest.Run(cacheEntry);
    // --- RUNNING TEST ---
    
    Net::NetCore::Instance()->DestroyAllControllersBlocked();
    Net::NetCore::Instance()->UnregisterAllServices();
    
    SafeDelete(client);
    SafeDelete(server);
}
    
}; // end of namespace AssetCache
}; // end of namespace DAVA

