#include "LightNode.hpp"

using namespace std;

LightNode::LightNode(Communicator::NodeType _nodeType,
	shared_ptr<LightStrip> _strip)
	:	strip(_strip)
	,	comm(_nodeType, strip->getSize(),
		std::bind(&LightNode::lightUpdate, this, std::placeholders::_1)) {
}

void LightNode::lightUpdate(const vector<Color>& _colors) {
	strip->setColors(_colors);
	strip->update();
}
