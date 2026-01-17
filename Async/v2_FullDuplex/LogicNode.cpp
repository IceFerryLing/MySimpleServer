#include "LogicNode.h"

LogicNode::LogicNode(shared_ptr<Session> session, shared_ptr<RecvNode> recvNode)
:_session(session),
_recvNode(recvNode)
{

}