#include "stakeholder_model.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Salience computation per Mitchell, Agle & Wood (1997):
 * salience = power * 0.4 + legitimacy * 0.3 + urgency * 0.3 */
double stk_salience_mitchell_1997(double power, double legitimacy, double urgency) {
    return 0.4 * power + 0.3 * legitimacy + 0.3 * urgency;
}

/* Stakeholder commitment assessment (Ulrich, 1983).
 * Commitment = willingness to engage × ability to influence decision.
 * Returns score in [0,1] where 1 = fully committed champion. */
double stk_commitment_index(const Stakeholder* s) {
    if (!s) return 0.0;
    double willingness = (s->interest > 0.3) ? s->interest : 0.3;
    double ability = s->influence * s->power;
    return willingness * ability;
}
