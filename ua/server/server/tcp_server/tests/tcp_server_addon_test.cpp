/// @author Alexander Rykovanov 2013
/// @email rykovanov.as@gmail.com
/// @brief Test of opc ua binary handshake.
/// @license GNU LGPL
///
/// Distributed under the GNU LGPL License
/// (See accompanying file LICENSE or copy at
/// http://www.gnu.org/licenses/lgpl.html)
///

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <opc/common/addons_core/addon_manager.h>
#include <opc/ua/client/remote_connection.h>
#include <opc/ua/server/addons/tcp_server_addon.h>

#include "../src/tcp_server_factory.h"

#include <chrono>
#include <thread>

using namespace testing;

const unsigned short TestPort = 33449;


namespace
{

  class EchoProcessor : public OpcUa::Server::IncomingConnectionProcessor
  {
  public:
    virtual void Process(std::shared_ptr<OpcUa::IOChannel> clientChannel)
    {
      char data[4] = {0};
      clientChannel->Receive(data, 4);
      clientChannel->Send(data, 4);
    }
    virtual void StopProcessing(std::shared_ptr<OpcUa::IOChannel> clientChannel)
    {
    }
  };

  class EchoAddon : public Common::Addon
  {
  public:
    EchoAddon()
    {
    }

  public:
    virtual void Initialize(Common::AddonsManager& addons, const Common::AddonParameters&)
    {
      std::shared_ptr<OpcUa::Server::IncomingConnectionProcessor> processor(new EchoProcessor());
      TcpAddon = Common::GetAddon<OpcUa::Server::TcpServerAddon>(addons, OpcUa::Server::TcpServerAddonID);
      OpcUa::Server::TcpParameters tcpParams;
      tcpParams.Port = 4841;

      TcpAddon->Listen(tcpParams, processor);
    }

    virtual void Stop()
    {
      OpcUa::Server::TcpParameters params;
      params.Port = 4841;
      TcpAddon->StopListen(params);
    }

  private:
    std::shared_ptr<OpcUa::Server::TcpServerAddon> TcpAddon;
  };

  class EchoAddonFactory : public Common::AddonFactory
  {
  public:
    /// @brief Create instance of addon
    virtual Common::Addon::UniquePtr CreateAddon()
    {
      return Common::Addon::UniquePtr(new EchoAddon());
    }
  };

  Common::AddonFactory::UniquePtr CreateEchoAddonFactory()
  {
    return Common::AddonFactory::UniquePtr(new EchoAddonFactory());
  }


  Common::AddonConfiguration CreateEchoAddonConfiguration()
  {
    Common::AddonConfiguration config;
    config.ID = "echo_addon";
    config.Factory =  CreateEchoAddonFactory();
    config.Dependencies = std::vector<Common::AddonID>(1, OpcUa::Server::TcpServerAddonID);
    return config;
  }

  Common::AddonConfiguration CreateTcpAddonConfig()
  {
    Common::AddonConfiguration tcpConfig;
    tcpConfig.ID = OpcUa::Server::TcpServerAddonID;
    tcpConfig.Factory.reset(new OpcUa::Impl::TcpServerFactory());
    return tcpConfig;
  }

}

TEST(TcpServerAddon, CanBeCreated)
{
  std::shared_ptr<Common::AddonsManager> addons = Common::CreateAddonsManager();
  addons->Register(CreateTcpAddonConfig());
  addons->Register(CreateEchoAddonConfiguration());
  addons->Start();
  ASSERT_TRUE(static_cast<bool>(addons->GetAddon(OpcUa::Server::TcpServerAddonID)));
  addons->Stop();
}

TEST(TcpServerAddon, CanSendAndReceiveData)
{
  std::shared_ptr<Common::AddonsManager> addons = Common::CreateAddonsManager();
  addons->Register(CreateTcpAddonConfig());
  addons->Register(CreateEchoAddonConfiguration());
  addons->Start();

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  std::unique_ptr<OpcUa::RemoteConnection> connection = OpcUa::Connect("localhost", 4841);

  char data[4] = {0, 1, 2, 3};
  connection->Send(data, 4);
  char dataReceived[4] = {0};
  connection->Receive(dataReceived, 4);

  ASSERT_EQ(memcmp(data, dataReceived, 4), 0);

  connection.reset();

  addons->Stop();
  addons.reset();
}
