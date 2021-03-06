//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseParsedVectorFunction.h"
#include "MooseParsedFunctionWrapper.h"

template <>
InputParameters
validParams<MooseParsedVectorFunction>()
{
  InputParameters params = validParams<Function>();
  params += validParams<MooseParsedFunctionBase>();
  params.addParam<std::string>("value_x", "0", "x-component of function.");
  params.addParam<std::string>("value_y", "0", "y-component of function.");
  params.addParam<std::string>("value_z", "0", "z-component of function.");
  return params;
}

MooseParsedVectorFunction::MooseParsedVectorFunction(const InputParameters & parameters)
  : Function(parameters),
    MooseParsedFunctionBase(parameters),
    _vector_value(verifyFunction(std::string("{") + getParam<std::string>("value_x") + "}{" +
                                 getParam<std::string>("value_y") + "}{" +
                                 getParam<std::string>("value_z") + "}"))
{
}

RealVectorValue
MooseParsedVectorFunction::vectorValue(Real t, const Point & p)
{
  return _function_ptr->evaluate<RealVectorValue>(t, p);
}

RealGradient
MooseParsedVectorFunction::gradient(Real /*t*/, const Point & /*p*/)
{
  mooseError("The gradient method is not defined in MooseParsedVectorFunction");
}

void
MooseParsedVectorFunction::initialSetup()
{
  if (!_function_ptr)
  {
    THREAD_ID tid = 0;
    if (isParamValid("_tid"))
      tid = getParam<THREAD_ID>("_tid");

    _function_ptr = libmesh_make_unique<MooseParsedFunctionWrapper>(
        _pfb_feproblem, _vector_value, _vars, _vals, tid);
  }
}
