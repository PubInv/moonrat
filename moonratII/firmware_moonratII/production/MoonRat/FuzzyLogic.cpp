//  Moonrat Control Code
//  Copyright (C) 2024 Robert L. Read, Horacio García-Vázquez
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <Fuzzy.h>

// fuzzy
Fuzzy *fuzzy = new Fuzzy();

// fuzzyInput(Error)
FuzzySet *negative = new FuzzySet(-50, -50, -5,0);
FuzzySet *zero = new FuzzySet(-3, 0, 0, 3);
FuzzySet *positive = new FuzzySet(0, 5, 50, 50);

// fuzzyInput(DiffError)
FuzzySet *dnegative = new FuzzySet(-50, -50, -2, 0);
FuzzySet *dzero = new FuzzySet(-1, 0, 0, 1);
FuzzySet *dpositive = new FuzzySet(0, 2, 50, 50);

// fuzzyOutput(Frequency)
FuzzySet *fast = new FuzzySet(10, 200, 255, 255);
FuzzySet *average = new FuzzySet(0, 5, 5, 20);
FuzzySet *slow = new FuzzySet(0, 0, 1, 5);

void setupFuzzy() {
  // fuzzyInput (Error)
  FuzzyInput *Error = new FuzzyInput(1);

  Error->addFuzzySet(negative);
  Error->addFuzzySet(zero);
  Error->addFuzzySet(positive);
  fuzzy->addFuzzyInput(Error);

  // fuzzyInput (DiffError)
  FuzzyInput *DiffError = new FuzzyInput(2);

  DiffError->addFuzzySet(dnegative);
  DiffError->addFuzzySet(dzero);
  DiffError->addFuzzySet(dpositive);
  fuzzy->addFuzzyInput(DiffError);

  // fuzzyOutput (Frequency)
  FuzzyOutput *Frequency = new FuzzyOutput(1);

  Frequency->addFuzzySet(fast);
  Frequency->addFuzzySet(average);
  Frequency->addFuzzySet(slow);
  fuzzy->addFuzzyOutput(Frequency);

  // Building fuzzy Rules
  FuzzyRuleAntecedent *errornegativeAndderrornegative = new FuzzyRuleAntecedent();
  errornegativeAndderrornegative->joinWithAND(negative, dnegative);
  FuzzyRuleAntecedent *errornegativeAndderrorzero = new FuzzyRuleAntecedent();
  errornegativeAndderrorzero->joinWithAND(negative, dzero);
  FuzzyRuleAntecedent *errorzeroAndderrornegative = new FuzzyRuleAntecedent();
  errorzeroAndderrornegative->joinWithAND(zero, dnegative);
  FuzzyRuleAntecedent *ifOutputEqualtoFastpartial = new FuzzyRuleAntecedent();
  ifOutputEqualtoFastpartial->joinWithOR( errornegativeAndderrornegative, errornegativeAndderrorzero);
  FuzzyRuleAntecedent *ifOutputEqualtoFast = new FuzzyRuleAntecedent();
  ifOutputEqualtoFast->joinWithOR( ifOutputEqualtoFastpartial, errorzeroAndderrornegative);
  FuzzyRuleConsequent *thenpwmfast = new FuzzyRuleConsequent();
  thenpwmfast->addOutput(fast);
  FuzzyRule *fuzzyRule01 = new FuzzyRule(1, ifOutputEqualtoFast, thenpwmfast);
  fuzzy->addFuzzyRule(fuzzyRule01);


  FuzzyRuleAntecedent *errorpositiveAndderrornegative = new FuzzyRuleAntecedent();
  errorpositiveAndderrornegative->joinWithAND(positive, dnegative);
  FuzzyRuleAntecedent *errorzeroAndderrorzero = new FuzzyRuleAntecedent();
  errorzeroAndderrorzero->joinWithAND(zero, dzero);
  FuzzyRuleAntecedent *errornegativeAndderrorpositive = new FuzzyRuleAntecedent();
  errornegativeAndderrorpositive->joinWithAND(negative, dpositive);
  FuzzyRuleAntecedent *ifOutputEqualtoAveragePartial = new FuzzyRuleAntecedent();
  ifOutputEqualtoAveragePartial ->joinWithOR( errorpositiveAndderrornegative, errorzeroAndderrorzero);
  FuzzyRuleAntecedent *ifOutputEqualtoAverage = new FuzzyRuleAntecedent();
  ifOutputEqualtoAverage ->joinWithOR( ifOutputEqualtoAveragePartial, errornegativeAndderrorpositive);
  FuzzyRuleConsequent *thenpwmAverage = new FuzzyRuleConsequent();
  thenpwmAverage->addOutput(average);
  FuzzyRule *fuzzyRule02 = new FuzzyRule(2, ifOutputEqualtoAverage, thenpwmAverage);
  fuzzy->addFuzzyRule(fuzzyRule02);


  FuzzyRuleAntecedent *errorpositiveAndderrorzero = new FuzzyRuleAntecedent();
  errorpositiveAndderrorzero->joinWithAND(positive, dzero);
  FuzzyRuleAntecedent *errorzeroAndderrorpositive = new FuzzyRuleAntecedent();
  errorzeroAndderrorpositive->joinWithAND(zero, dpositive);
  FuzzyRuleAntecedent *errorpositiveAndderrorpositive = new FuzzyRuleAntecedent();
  errorpositiveAndderrorpositive->joinWithAND(positive, dpositive);
  FuzzyRuleAntecedent *ifOutputEqualtoSlowPartial = new FuzzyRuleAntecedent();
  ifOutputEqualtoSlowPartial ->joinWithOR( errorpositiveAndderrorzero, errorzeroAndderrorpositive);
  FuzzyRuleAntecedent *ifOutputEqualtoSlow = new FuzzyRuleAntecedent();
  ifOutputEqualtoSlow ->joinWithOR(ifOutputEqualtoSlowPartial, errorpositiveAndderrorpositive);
  FuzzyRuleConsequent *thenpwmslow = new FuzzyRuleConsequent();
  // Including a FuzzySet to this fuzzyRuleConsequent
  thenpwmslow->addOutput(slow);
  // Instantiating a fuzzyRule objects
  FuzzyRule *fuzzyRule03 = new FuzzyRule(3, ifOutputEqualtoSlow, thenpwmslow);
  // Including the fuzzyRule into fuzzy
  fuzzy->addFuzzyRule(fuzzyRule03);

}
