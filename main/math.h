// *********************************************************
// math.h
// copyright (c) JE all rights reserved
// 2010/05/31
// *********************************************************

#ifndef COMMON_MATH_H
#define COMMON_MATH_H

#include <cmath>
#include <algorithm>

namespace math
{

template< typename T > inline T clamp( T value, T min_value, T max_value )
{
	value = std::max( min_value, value );
	value = std::min( max_value, value );

	return value;
}

template< typename T > inline T degree_to_radian( T degree )
{
	return degree * ( static_cast< float >( M_PI ) / 180 );
}

template< typename T > inline T radian_to_degree( T radian )
{
	return radian * ( 180 / static_cast< float >( M_PI ) );
}

template< typename T > inline T chase( T value, T target, T speed )
{
	if ( value < target )
	{
		value += speed;

		if ( value > target )
		{
			value = target;
		}
	}
	else if ( value > target )
	{	
		value -= speed;

		if ( value < target )
		{
			value = target;
		}
	}

	return value;
}

} // namespace math

#endif // COMMON_MATH_H
