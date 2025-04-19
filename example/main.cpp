
#include "../include/engine/engine.h"
#include "../include/engine/transaction.h"
int main()
{
    LSM engine("temp_dir");
    auto handler = engine.begin_transaction(IsolationLevel::RepeatableRead);

    handler->put("key", "value");
    handler->remove("key");

    handler->commit();
}