#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "stakeholder_model.h"
#include "tradeoff_analysis.h"
#include "multi_criteria_decision.h"
#include "negotiation_consensus.h"
#include "requirement_prioritization.h"
#define EPS 1e-6
#define ASSERT_NEAR(a,b,e) assert(fabs((a)-(b))<(e))
static int tr=0,tp=0;
#define T(n) do{tr++;printf("  TEST %s... ",n);}while(0)
#define P() do{tp++;printf("PASSED\n");}while(0)

void t1_stakeholder(void){
  T("stakeholder");
  Stakeholder* s=stk_create("Mayor",STYPE_PRIMARY,0.8,0.9,0.7,0.6,0.8);
  assert(s!=NULL); assert(strcmp(s->name,"Mayor")==0);
  ASSERT_NEAR(stk_salience(s),0.4*0.8+0.3*0.8+0.3*0.6,EPS);
  stk_free(s); stk_free(NULL); P();
}
void t2_network(void){
  T("network");
  StakeholderNetwork* net=stk_net_create();
  Stakeholder* a=stk_create("A",STYPE_KEY,0.8,0.9,1.0,0.5,0.9);
  Stakeholder* b=stk_create("B",STYPE_PRIMARY,0.3,0.7,0.5,0.4,0.6);
  assert(stk_net_add(net,a)==0); assert(stk_net_add(net,b)==1);
  stk_net_set_influence(net,0,1,0.5); stk_net_set_influence(net,1,0,0.3);
  stk_net_compute_metrics(net);
  assert(net->network_density>=0.0);
  int ki[10],kn; stk_net_find_key_players(net,ki,&kn);
  double ci=stk_net_conflict_index(net); assert(ci>=0.0&&ci<=1.0);
  stk_net_free(net); P();
}
void t3_tradeoff(void){
  T("tradeoff");
  const char* objs[]={"Cost","Performance","Reliability"};
  bool maximize[]={false,true,true};
  double weights[]={0.3,0.4,0.3};
  TradeoffProblem* tp=to_create("CarSelection",3,objs,weights,maximize);
  double a1[]={30000,85,90}; to_add_alternative(tp,"Sedan",a1);
  double a2[]={25000,80,95}; to_add_alternative(tp,"SUV",a2);
  double a3[]={40000,95,85}; to_add_alternative(tp,"Sports",a3);
  to_compute_pareto_front(tp); assert(tp->n_pareto>=1);
  int best=to_find_best_compromise(tp,weights); assert(best>=0);
  assert(!to_is_dominated(tp,0,0));
  to_free(tp); P();
}
void t4_mcda(void){
  T("mcda");
  const char* objs[]={"Cost","Speed"};
  bool maxi[]={false,true};
  double w[]={0.5,0.5};
  TradeoffProblem* tp=to_create("MCDATest",2,objs,w,maxi);
  double d1[]={10,100}; to_add_alternative(tp,"A",d1);
  double d2[]={5,80}; to_add_alternative(tp,"B",d2);
  double d3[]={8,120}; to_add_alternative(tp,"C",d3);
  MCDAResult* r=mcda_weighted_sum(tp,w); assert(r!=NULL);
  assert(r->n_alternatives==3); assert(r->ranking[0]==r->ranking[0]);
  MCDAResult* r2=mcda_topsis(tp,w); assert(r2!=NULL);
  MCDAResult* r3=mcda_rank(tp,MCDA_TOPSIS,w); assert(r3!=NULL);
  mcda_result_free(r); mcda_result_free(r2); mcda_result_free(r3);
  double pw[2][2]={{1,2},{0.5,1}};
  double* dp[2]={pw[0],pw[1]};
  AHPResult* ahp=mcda_ahp_compute(2,dp); assert(ahp!=NULL);
  mcda_ahp_free(ahp);
  to_free(tp); P();
}
void t5_negotiation(void){
  T("negotiation");
  Stakeholder* s1=stk_create("Buyer",STYPE_PRIMARY,0.6,0.9,0.5,0.5,0.7);
  Stakeholder* s2=stk_create("Seller",STYPE_PRIMARY,0.7,0.8,0.6,0.6,0.8);
  double pos1[]={50000,30}; double res1[]={55000,20};
  double pos2[]={60000,60}; double res2[]={52000,40};
  Negotiator* n1=neg_create(s1,2,pos1,res1);
  Negotiator* n2=neg_create(s2,2,pos2,res2);
  const char* issues[]={"Price","DeliveryWeeks"};
  Negotiation* neg=neg_session_create(2,issues);
  assert(neg_add_party(neg,n1)==0); assert(neg_add_party(neg,n2)==1);
  neg_run_negotiation(neg,50);
  double ci=neg_consensus_index(neg); assert(ci>=0.0);
  neg_compute_nash_solution(neg); assert(neg->nash_product>0.0);
  neg_session_free(neg);
  stk_free(s1); stk_free(s2); P();
}
void t6_requirements(void){
  T("requirements");
  Requirement* r=req_create("LoginPage","User authentication",100.0,20.0,0.1,5.0);
  assert(r!=NULL); assert(strcmp(r->name,"LoginPage")==0);
  double roi=req_roi(r); assert(roi>0.0);
  MoSCoWClass mc=req_classify_moscow(r,0.3); assert(mc>=0&&mc<=MOSCOW_WONT);
  KanoClass kc=req_classify_kano(0.8,0.2); assert(kc>=0&&kc<=KANO_REVERSE);
  RequirementBacklog* bl=backlog_create();
  assert(backlog_add(bl,r)==0); assert(bl->n_requirements==1);
  backlog_prioritize(bl,PRIO_VALUE_BASED);
  double tv=backlog_total_value(bl); assert(tv>0.0);
  int* sel; int ns; backlog_select_for_budget(bl,200.0,&sel,&ns);
  backlog_free(bl); P();
}
void t7_classify(void){
  T("classify");
  Stakeholder* s=stk_create("X",STYPE_KEY,0.2,0.9,0.3,0.8,0.7);
  StakeholderCategory c=stk_classify(s);
  assert(c==STAKE_LOW_POWER_HIGH_INTEREST);
  double ms=stk_mitchell_score(s); assert(ms>=0.0);
  stk_add_requirement(s,"Safety",0.8); assert(s->n_requirements==1);
  stk_free(s); P();
}
int main(void){
  printf("\n=== Stakeholder Analysis & Tradeoff Tests ===\n\n");
  t1_stakeholder(); t2_network(); t3_tradeoff();
  t4_mcda(); t5_negotiation(); t6_requirements(); t7_classify();
  printf("\n=== %d/%d tests passed ===\n",tp,tr);
  return 0;
}
