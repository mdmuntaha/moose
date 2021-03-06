//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactSplit.h"
#include "InputParameters.h"
#include "FEProblem.h"

#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3, 3, 0)
template <>
InputParameters
validParams<ContactSplit>()
{
  InputParameters params = validParams<Split>();
  params.addParam<std::vector<std::string>>("contact_master",
                                            "Master surface list for included contacts");
  params.addParam<std::vector<std::string>>("contact_slave",
                                            "Slave surface list for included contacts");
  params.addParam<std::vector<int>>(
      "contact_displaced",
      "List of indicators whether displaced mesh is used to define included contact");
  params.addParam<std::vector<std::string>>("uncontact_master",
                                            "Master surface list for excluded contacts");
  params.addParam<std::vector<std::string>>("uncontact_slave",
                                            "Slave surface list for excluded contacts");
  params.addParam<std::vector<int>>(
      "uncontact_displaced",
      "List of indicators whether displaced mesh is used to define excluded contact");
  // Right now, we consider this as a required parameter.
  // After some tests from BISON, we will set a default value for this parameter.
  params.addRequiredParam<bool>("include_all_contact_nodes",
                                "Whether to include all nodes on the contact surfaces");
  return params;
}

ContactSplit::ContactSplit(const InputParameters & params)
  : Split(params),
    _contact_master(getParam<std::vector<std::string>>("contact_master")),
    _contact_slave(getParam<std::vector<std::string>>("contact_slave")),
    _contact_displaced(getParam<std::vector<int>>("contact_displaced")),
    _uncontact_master(getParam<std::vector<std::string>>("uncontact_master")),
    _uncontact_slave(getParam<std::vector<std::string>>("uncontact_slave")),
    _uncontact_displaced(getParam<std::vector<int>>("uncontact_displaced")),
    _include_all_contact_nodes(getParam<bool>("include_all_contact_nodes"))
{
  if (_contact_master.size() != _contact_slave.size())
  {
    std::ostringstream err;
    err << "Master and slave contact lists must have matching sizes: " << _contact_master.size()
        << " != " << _contact_slave.size();
    mooseError(err.str());
  }
  if (_contact_displaced.size() && _contact_master.size() != _contact_displaced.size())
  {
    std::ostringstream err;
    err << "Master and displaced contact lists must have matching sizes: " << _contact_master.size()
        << " != " << _contact_displaced.size();
    mooseError(err.str());
  }
  if (!_contact_displaced.size())
    _contact_displaced.resize(_contact_master.size());

  if (_uncontact_master.size() != _uncontact_slave.size())
  {
    std::ostringstream err;
    err << "Master and slave uncontact lists must have matching sizes: " << _uncontact_master.size()
        << " != " << _uncontact_slave.size();
    mooseError(err.str());
  }
  if (_uncontact_displaced.size() && _uncontact_master.size() != _uncontact_displaced.size())
  {
    std::ostringstream err;
    err << "Master and displaced uncontact lists must have matching sizes: "
        << _uncontact_master.size() << " != " << _uncontact_displaced.size();
    mooseError(err.str());
  }
  if (!_uncontact_displaced.size())
    _uncontact_displaced.resize(_uncontact_master.size());
}

void
ContactSplit::setup(const std::string & prefix)
{
  // A reference to the PetscOptions
  Moose::PetscSupport::PetscOptions & po = _fe_problem.getPetscOptions();
  // prefix
  std::string dmprefix = prefix + "dm_moose_", opt, val;

  // contacts options
  if (_contact_master.size())
  {
    opt = dmprefix + "ncontacts";
    {
      std::ostringstream oval;
      oval << _contact_master.size();
      val = oval.str();
    }
    // push back PETSc options
    if (val == "")
      po.flags.push_back(opt);
    else
    {
      po.inames.push_back(opt);
      po.values.push_back(val);
    }
    for (unsigned int j = 0; j < _contact_master.size(); ++j)
    {
      std::ostringstream oopt;
      oopt << dmprefix << "contact_" << j;
      opt = oopt.str();
      val = _contact_master[j] + "," + _contact_slave[j];
      // push back PETSc options
      if (val == "")
        po.flags.push_back(opt);
      else
      {
        po.inames.push_back(opt);
        po.values.push_back(val);
      }
      if (_contact_displaced[j])
      {
        opt = opt + "_displaced";
        val = "yes";
        // push back PETSc options
        if (val == "")
          po.flags.push_back(opt);
        else
        {
          po.inames.push_back(opt);
          po.values.push_back(val);
        }
      }
    }
  }
  // uncontacts options
  if (_uncontact_master.size())
  {
    opt = dmprefix + "nuncontacts";
    {
      std::ostringstream oval;
      oval << _uncontact_master.size();
      val = oval.str();
    }
    // push back PETSc options
    if (val == "")
      po.flags.push_back(opt);
    else
    {
      po.inames.push_back(opt);
      po.values.push_back(val);
    }
    for (unsigned int j = 0; j < _uncontact_master.size(); ++j)
    {
      std::ostringstream oopt;
      oopt << dmprefix << "uncontact_" << j;
      opt = oopt.str();
      val = _uncontact_master[j] + "," + _uncontact_slave[j];
      // push back PETSc options
      if (val == "")
        po.flags.push_back(opt);
      else
      {
        po.inames.push_back(opt);
        po.values.push_back(val);
      }
      if (_uncontact_displaced[j])
      {
        opt = opt + "_displaced";
        val = "yes";
        // push back PETSc options
        if (val == "")
          po.flags.push_back(opt);
        else
        {
          po.inames.push_back(opt);
          po.values.push_back(val);
        }
      }
    }
  }

  // Whether to include all nodes on the contact surfaces
  // into the contact subsolver
  opt = dmprefix + "includeAllContactNodes";
  if (_include_all_contact_nodes)
    val = "yes";
  else
    val = "no";
  po.inames.push_back(opt);
  po.values.push_back(val);
  Split::setup(prefix);
}
#endif
