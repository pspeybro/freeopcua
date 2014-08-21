/// @author Alexander Rykovanov 2013
/// @email rykovanov.as@gmail.com
/// @brief Remote Computer implementaion.
/// @license GNU GPL
///
/// Distributed under the GNU GPL License
/// (See accompanying file LICENSE or copy at
/// http://www.gnu.org/licenses/gpl.html)
///



#include <opc/ua/client/client.h>
#include <opc/ua/node.h>
#include <opc/ua/subscription.h>

#include <iostream>
#include <stdexcept>
#ifdef _WIN32
#else
#include <unistd.h>
#endif

using namespace OpcUa;

class SubClient : public SubscriptionClient
{
  void DataChange(uint32_t handle, const Node& node, const Variant& val, AttributeID attr) override
  {
    std::cout << "Received DataChange event, value of Node " << node << " is now: "  << std::endl;
  }
};

int main(int argc, char** argv)
{
  try
  {
      //std::string endpoint = "opc.tcp://192.168.56.101:48030";
      std::string endpoint = "opc.tcp://127.0.0.1:4841";

      std::cout << "Connecting to: " << endpoint << std::endl;
      OpcUa::RemoteClient client(endpoint);
      client.Connect();

      OpcUa::Node root = client.GetRoot();
      std::cout << "Root node is: " << root << std::endl;
      std::vector<std::string> path({"Objects", "Server"});
      OpcUa::Node server = root.GetChild(path);
      std::cout << "Server node obtained by path: " << server << std::endl;

      std::cout << "Child of objects node are: " << std::endl;
      for (OpcUa::Node node : client.GetObjectsFolder().GetChildren())
        std::cout << "    " << node << std::endl;

      std::cout << "NamespaceArray is: " << std::endl;
      std::vector<std::string> nspath ({"Objects", "Server", "NamespaceArray"});
      OpcUa::Node nsnode = root.GetChild(nspath);
      OpcUa::Variant ns  = nsnode.GetValue();

      for (std::string d : ns.Value.String)
        std::cout << "    "  << d << std::endl;



      /*
      std::vector<std::string> nspath ({"Objects", "Server", "NamespaceArray"});
        OpcUa::Node node = serv.GetChildNode("ServiceLevel");
        if (nsnode) {
          std::cout << "ServiceLevel is: " << node << std::endl;
            OpcUa::Variant ns  = node.ReadValue();
            for (uint d : ns.Value.Byte) {

                std::cout << "ServiceLevel is: "  << d << std::endl;
            }
        }
      }
      */


      //Subscription
      std::vector<std::string> varpath({"Objects", "testfolder", "myvar"});
      OpcUa::Node myvar = root.GetChild(varpath);
      std::cout << "got node: " << myvar << std::endl;
      SubClient sclt; 
      Subscription sub = client.CreateSubscription(100, sclt);
      uint32_t handle = sub.SubscribeDataChange(myvar);
      std::cout << "Got sub handle: " << handle << ", sleeping Xs" << std::endl;
      sleep(10);


      std::cout << "Disconnecting" << std::endl;
      return 0;
  }
  catch (const std::exception& exc)
  {
    std::cout << exc.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "Unknown error." << std::endl;
  }
  return -1;
}

