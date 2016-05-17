/*
  ==============================================================================

    Rule.h
    Created: 4 May 2016 5:06:55pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef RULE_H_INCLUDED
#define RULE_H_INCLUDED

#include "JuceHeader.h"
#include "RuleCondition.h"
#include "RuleConditionGroup.h"

#include "ScriptedCondition.h"


#include "ControllableContainer.h"
#include "ControlVariableReference.h"

class RuleConsequence;


class Rule :
	public ControllableContainer,
	public ControlVariableReference::ControlVariableReferenceListener,
	public RuleCondition::RuleConditionListener,
	public RuleConditionGroupListener
{
public:
	enum ActivationType
	{
		OnActivate,
		OnDeactivate,
		WhileActivated,
		WhileDeactivated
	};

	Rule(const String &name);
	virtual ~Rule();

	enum ConditionType {NONE, SCRIPT, VISUAL };
	ConditionType conditionType;
	void setConditionType(ConditionType value);

	StringParameter * nameParam;
	BoolParameter * enabledParam;
	BoolParameter * isActiveParam;

	ActivationType activationType;

	OwnedArray<ControlVariableReference> references;
	ScopedPointer<RuleConditionGroup> rootConditionGroup;
	ScopedPointer<ScriptedCondition> scriptedCondition;

	OwnedArray<RuleConsequence> consequences;

	ControlVariableReference * addReference();
	void removeReference(ControlVariableReference *);

	void updateReferencesInCondition();
	void updateReferencesInConsequences();

	void addConsequence();
	void removeConsequence(RuleConsequence *);

	void onContainerParameterChanged(Parameter * p) override;
	void askForRemoveReference(ControlVariableReference * r) override;


	void conditionActivationChanged(RuleCondition * c) override;
	
	bool isActive();

	void remove();


	class  RuleListener
	{
	public:
		virtual ~RuleListener() {}

		virtual void askForRemoveRule(Rule *) {}
		virtual void ruleActivationChanged(Rule *) {}
		virtual void ruleConditionTypeChanged(Rule *) {}
		virtual void referenceAdded(ControlVariableReference *) {}
		virtual void referenceRemoved(ControlVariableReference *) {}

		virtual void consequenceAdded(RuleConsequence *) {}
		virtual void consequenceRemoved(RuleConsequence *) {}


	};


	ListenerList<RuleListener> ruleListeners;
	void addRuleListener(RuleListener* newListener) { ruleListeners.add(newListener); }
	void removeRuleListener(RuleListener* listener) { ruleListeners.remove(listener); }

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Rule)
};



#endif  // RULE_H_INCLUDED
