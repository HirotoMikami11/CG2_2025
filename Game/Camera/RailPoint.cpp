#include "RailPoint.h"
#include <format>

// 静的メンバの初期化
int RailPoint::nextId_ = 0;

RailPoint::RailPoint(const Vector3& position, const std::string& name)
	: position_(position)
	, name_(name.empty() ? std::format("Point_{}", nextId_) : name)
	, id_(nextId_++)
{}