#pragma once
#include "Singleton.h"
#include <queue>
#include <thread>
#include "Session_demo.h"
#include <map>
#include <functional>
#include "const.h"
#include "json/json.h"
#include "json/value.h"
#include "json/reader.h"

class LogicNode{
    friend class LogicSystem;

public:
    LogicNode(shared_ptr<Session>, shared_ptr<RecvNode>);

private:
    shared_ptr<Session> _session;
    shared_ptr<RecvNode> _recvnode;
};