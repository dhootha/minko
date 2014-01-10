/*
Copyright (c) 2014 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "minko/animation/Matrix4x4Timeline.hpp"
#include "minko/data/Container.hpp"
#include "minko/math/Matrix4x4.hpp"
#include "timeline_lookup.hpp"

using namespace minko;
using namespace minko::component;
using namespace minko::animation;
using namespace minko::math;

Matrix4x4Timeline::Matrix4x4Timeline(const std::string& propertyName,
									 uint duration,
									 const std::vector<uint>& timetable,
									 const std::vector<Matrix4x4Ptr>& matrices,
									 bool interpolate,
									 bool interpolateScale,
									 bool interpolateTranslation):
	AbstractTimeline(propertyName, duration),
	_matrices(),
	_interpolate(interpolate),
	_interpolateScale(interpolateScale),
	_interpolateTranslation(interpolateTranslation)
{
	initializeMatrixTimetable(timetable, matrices);
}

void
Matrix4x4Timeline::initializeMatrixTimetable(const std::vector<uint>& timetable,
											 const std::vector<Matrix4x4Ptr>& matrices)
{
	if (timetable.empty())
		throw new std::invalid_argument("timetable");
	if (matrices.empty())
		throw new std::invalid_argument("matrices");
	if (timetable.size() != matrices.size())
		throw new std::logic_error("The number of keys must match in both the 'timetable' and 'matrices' parameters.");

	const uint numKeys = timetable.size();

	_matrices.clear();
	_matrices.resize(numKeys);
	for (uint keyId = 0; keyId < numKeys; ++keyId)
	{
		_matrices[keyId].first	= timetable[keyId];
		_matrices[keyId].second	= matrices[keyId];
	}

	std::sort(_matrices.begin(), _matrices.end());
}

void
Matrix4x4Timeline::update(uint time, 
						  UpdateTargetPtr data, 
						  bool /*skipPropertyNameFormatting*/)
{
	if (_isLocked || _duration == 0 || _matrices.empty())
		return;

	if (data == nullptr || !data->hasProperty(_propertyName))
		return;


	return;
	auto matrix	= data->get<Matrix4x4::Ptr>(_propertyName);

	if (matrix == nullptr)
		return;

	const uint	t		= getTimeInRange(time, _duration + 1);
	const uint	keyId	= getIndexForTime(t, _matrices);

	const bool interpolate = _interpolate 
		&& (t >= _matrices.front().first) 
		&& (t < _matrices.back().first);

	if (interpolate)
	{
		throw new std::logic_error("matrix decomposition and interpolation not implemented yet!");
	}
	else
	{
		//matrix->copyFrom(_matrices[keyId].second);

		std::cout << "time = " << time << "\t[" << keyId << "]\tmat = " << matrix->toString() << std::endl;
	}
}