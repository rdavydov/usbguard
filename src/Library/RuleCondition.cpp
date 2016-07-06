//
// Copyright (C) 2016 Red Hat, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Authors: Daniel Kopecek <dkopecek@redhat.com>
//
#include "RuleCondition.hpp"
#include "Rule.hpp"
#include "LoggerPrivate.hpp"

#include <unordered_map>
#include <functional>

namespace usbguard
{
  RuleCondition::RuleCondition(const String& identifier, const String& parameter, bool negated)
    : _identifier(identifier),
      _parameter(parameter),
      _negated(negated)
  {
  }

  RuleCondition::RuleCondition(const String& identifier, bool negated)
    : _identifier(identifier),
      _negated(negated)
  {
  }

  RuleCondition::RuleCondition(const RuleCondition& rhs)
    : _identifier(rhs._identifier),
      _parameter(rhs._parameter),
      _negated(rhs._negated)
  {
  }

  RuleCondition::~RuleCondition()
  {
    fini();
  }

  void RuleCondition::init(Interface * const interface_ptr)
  {
    (void)interface_ptr;
  }

  void RuleCondition::fini()
  {
  }

  bool RuleCondition::evaluate(const Rule& rule)
  {
    return isNegated() ? !update(rule) : update(rule);
  }

  const String& RuleCondition::identifier() const
  {
    return _identifier;
  }

  const String& RuleCondition::parameter() const
  {
    return _parameter;
  }

  bool RuleCondition::hasParameter() const
  {
    return !_parameter.empty();
  }

  bool RuleCondition::isNegated() const
  {
    return _negated;
  }

  const String RuleCondition::toString() const
  {
    String condition_string;
    
    if (isNegated()) {
      condition_string.append("!");
    }
    
    condition_string.append(identifier());
   
    if (hasParameter()) {
      condition_string.append("(");
      condition_string.append(parameter()); /* TODO: Escape parameter string */
      condition_string.append(")");
    }

    return condition_string;
  }

  const String RuleCondition::toRuleString() const
  {
    return toString();
  }
} /* namespace usbguard */

#include "AllowedMatchesCondition.hpp"
#include "LocaltimeCondition.hpp"
#include "FixedStateCondition.hpp"
#include "RandomStateCondition.hpp"
#include "RuleAppliedCondition.hpp"
#include "RuleEvaluatedCondition.hpp"
#include <iostream>

namespace usbguard
{
  RuleCondition* RuleCondition::getImplementation(const String& condition_string)
  {
    if (condition_string.empty()) {
      throw std::runtime_error("Empty condition");
    }
    const bool negated = condition_string[0] == '!';
    const size_t identifier_start = negated ? 1 : 0;
    const size_t p_pos = condition_string.find_first_of('(');

    String identifier;
    String parameter;

    if (p_pos == std::string::npos) {
      /*
       * The rest of the condition_string should be
       * a condition identifier -- without a parameter.
       */
      identifier = condition_string.substr(identifier_start);

      if (identifier.size() < 1) {
        throw std::runtime_error("Invalid condition string. Missing identifier.");
      }
    }
    else {
      const size_t parameter_size = condition_string.size() - p_pos;

      if (parameter_size < 3 /* two parentheses + at least one character */) {
        throw std::runtime_error("Invalid condition string. Invalid parameter.");
      }

      const size_t identifier_size = p_pos - identifier_start;
      identifier = condition_string.substr(identifier_start, identifier_size);

      if (condition_string[condition_string.size() - 1] != ')') {
        throw std::runtime_error("Invalid condition string. Malformed parameter.");
      }

      parameter = condition_string.substr(p_pos + 1, parameter_size - 2);
    }

    return getImplementation(identifier, parameter, negated);
  }

  RuleCondition* RuleCondition::getImplementation(const String& identifier, const String& parameter, bool negated)
  {
    if (identifier == "allowed-matches") {
      return new AllowedMatchesCondition(parameter, negated);
    }
    if (identifier == "localtime") {
      return new LocaltimeCondition(parameter, negated);
    }
    if (identifier == "true") {
      return new FixedStateCondition(true, negated);
    }
    if (identifier == "false") {
      return new FixedStateCondition(false, negated);
    }
    if (identifier == "random") {
      return new RandomStateCondition(parameter, negated);
    }
    if (identifier == "rule-applied") {
      return new RuleAppliedCondition(parameter, negated);
    }
    if (identifier == "rule-evaluated") {
      return new RuleEvaluatedCondition(parameter, negated);
    }
    throw std::runtime_error("Unknown rule condition");
  }
} /* namespace usbguard */

