#include <ffnetwork/client.h>
#include "curl_client.h"

namespace ffnetwork {
    Client::~Client() {

    }

    std::shared_ptr<Client> createClient() {
        return createCurlClient();
    }

}
