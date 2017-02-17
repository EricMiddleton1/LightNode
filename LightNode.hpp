#pragma once

#include <vector>
#include <memory>
#include <cstdint>

#include "Communicator.hpp"
#include "LightStrip.hpp"

class LightNode
{
public:
	LightNode(Communicator::NodeType, std::shared_ptr<LightStrip>);

private:
	void lightUpdate(const std::vector<Color>&);

	std::shared_ptr<LightStrip> strip;
	Communicator comm;
};
