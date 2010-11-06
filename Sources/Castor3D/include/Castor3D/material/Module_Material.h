/*
This source file is part of Castor3D (http://dragonjoker.co.cc

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
the program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
*/
#ifndef ___C3D_Module_Material___
#define ___C3D_Module_Material___

#include "../Module_General.h"

namespace Castor3D
{
	class TextureEnvironment;	//!< The texture environment class
	class TextureUnit;			//!< The texture unit class
	class Material;				//!< The material class
	class MaterialManager;		//!< The Material manager class
	class Pass;					//!< The material pass class (for multipass materials)

	//! Environment modes enumerator
	/*!
	The different environment modes for materials : Replace, Modulate, Blend, Add and Combine
	*/
	typedef enum EnvironmentMode
	{
		EMReplace	= 0,	//!< replace the previous texture
		EMModulate	= 1,	//!< modulate with the previous texture
		EMBlend		= 2,	//!< blend with the previous texture
		EMAdd		= 3,	//!< add to the previous texture
		EMCombine	= 4		//!< Combine with the previous texture, implies more configuration
	} EnvironmentMode;

	//! RGB Combinations enumerator
	/*!
	The different RGB combination modes : none, replace, modulate, add, add signed, dot 3 RGB, dot 3 RGBA, interpolate
	*/
	typedef enum RGBCombination
	{
		CCNone			= 0,	//!< None
		CCReplace		= 1,	//!< Arg0
		CCModulate		= 2,	//!< Arg0 * Arg1
		CCAdd			= 3,	//!< Arg0 + Arg1
		CCAddSigned		= 4,	//!< Arg0 + Arg1 - 0.5
		CCDot3RGB		= 5,	//!< 4 * ((Arg0r - 0.5) * (Arg1r - 0.5) + (Arg0g - 0.5) * (Arg1g - 0.5) + (Arg0b - 0.5) * (Arg1b - 0.5))
		CCDot3RGBA		= 6,	//!< 4 * ((Arg0r - 0.5) * (Arg1r - 0.5) + (Arg0g - 0.5) * (Arg1g - 0.5) + (Arg0b - 0.5) * (Arg1b - 0.5) + (Arg0a - 0.5) * (Arg1a - 0.5))
		CCInterpolate	= 7		//!< Arg0 * Arg2 + Arg1 * (1 - Arg2)
	} RGBCombination;

	//! RGB Operands enumerator
	/*!
	The different RGB operands that can be used in textures environment
	*/
	typedef enum RGBOperand
	{
		COSrcColour			= 0,	//!< Uses source colour
		COOneMinusSrcColour	= 1,	//!< Uses 1 - source colour
		COSrcAlpha			= 2,	//!< Uses source alpha
		COOneMinusSrcAlpha	= 3		//!< Uses 1 - source alpha
	} RGBOperand;

	//! Alpha Combinations enumerator
	/*!
	The different Alpha combination modes : none, replace, modulate, add, add signed, interpolate
	*/
	typedef enum AlphaCombination
	{
		ACNone			= 0,	//!< None
		ACReplace		= 1,	//!< Arg0
		ACModulate		= 2,	//!< Arg0 * Arg1
		ACAdd			= 3,	//!< Arg0 + Arg1
		ACAddSigned		= 4,	//!< Arg0 + Arg1 - 0.5
		ACInterpolate	= 5		//!< Arg0 * Arg2 + Arg1 * (1 - Arg2)
	} AlphaCombination;

	//! Alpha Operands enumerator
	/*!
	The different Alpha operands that can be used in textures environment
	*/
	typedef enum AlphaOperand
	{
		AOSrcAlpha			= 0,	//!< Uses source alpha
		AOOneMinusSrcAlpha	= 1		//!< Uses 1 - source alpha
	} AlphaOperand;

	//! Combination sources enumerator
	/*!
	The different combination sources are texture, current texture, constant, primary colour and previous
	*/
	typedef enum CombinationSource
	{
		CSTexture			= 0,	//!< The n-th texture unit is the source
		CSCurrentTexture	= 1,	//!< The current texture unit is the source
		CSConstant			= 2,	//!< The source is the defined constant colour
		CSPrimaryColour		= 3,	//!< The source is the fragment colour before texturing
		CSPrevious			= 4		//!< The source is the fragment colour before the texture
	} CombinationSource;


	//! Light indexes enumerator
	/*!
	The height available light indexes
	*/
	typedef enum LightIndexes
	{
		LILight0 = 0,
		LILight1 = 1,
		LILight2 = 2,
		LILight3 = 3,
		LILight4 = 4,
		LILight5 = 5,
		LILight6 = 6,
		LILight7 = 7
	} LightIndexes;

	typedef Templates::SharedPtr<TextureEnvironment>	TextureEnvironmentPtr;
	typedef Templates::SharedPtr<TextureUnit>			TextureUnitPtr;
	typedef Templates::SharedPtr<MaterialManager>		MaterialManagerPtr;
	typedef Templates::SharedPtr<Material>				MaterialPtr;
	typedef Templates::SharedPtr<Pass>					PassPtr;

	typedef C3DMap(		String, MaterialPtr)	MaterialPtrStrMap;		//! Material pointer map, sorted by name
	typedef C3DVector(	MaterialPtr)			MaterialPtrArray;		//! Material pointer array
	typedef C3DMap(		size_t, MaterialPtr)	MaterialPtrUIntMap;
	typedef C3DVector(	TextureUnitPtr)			TextureUnitPtrArray;	//! TextureUnit pointer array
	typedef C3DVector(	PassPtr)				PassPtrArray;			//! TextureUnit pointer array
	typedef C3DVector(	unsigned int)			UIntArray;				//! unsigned int array
}

#endif

