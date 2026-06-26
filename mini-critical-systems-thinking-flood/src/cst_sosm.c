#include "cst_sosm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

CSTSOSM* cst_sosm_create(CSTSystem* sys) {
    CSTSOSM* sosm = calloc(1, sizeof(CSTSOSM));
    if (!sosm) return NULL;
    sosm->sys = sys;
    const char* cell_names[3][3] = {
        {"Simple-Unitary", "Simple-Pluralist", "Simple-Coercive"},
        {"Complex-Unitary", "Complex-Pluralist", "Complex-Coercive"},
        {"Chaotic-Unitary", "Chaotic-Pluralist", "Chaotic-Coercive"}
    };
    const char* ideal_methods[3][3] = {
        {"Hard SE / OR", "SAST / SODA", "CSH / Boundary Critique"},
        {"System Dynamics / VSM", "Soft Systems Methodology", "TSI / CST"},
        {"Crisis Management", "Postmodern Deconstruction", "Emancipatory Action Research"}
    };
    const char* fallback_methods[3][3] = {
        {"Systems Analysis", "Interactive Planning", "Critical OR"},
        {"Agent-Based Modeling", "Strategic Choice", "Pluralist TSI"},
        {"Scenario Planning", "Narrative Methods", "Participatory Design"}
    };
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            sosm->grid[r][c].complexity = (CSTComplexity)r;
            sosm->grid[r][c].relationship = (CSTRelationship)c;
            sosm->grid[r][c].cell_name = strdup(cell_names[r][c]);
            sosm->grid[r][c].ideal_methodology = strdup(ideal_methods[r][c]);
            sosm->grid[r][c].fallback_methodology = strdup(fallback_methods[r][c]);
            sosm->grid[r][c].fit_score = 0.0;
        }
    }
    return sosm;
}

void cst_sosm_free(CSTSOSM* sosm) {
    if (!sosm) return;
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++) {
            free(sosm->grid[r][c].cell_name);
            free(sosm->grid[r][c].ideal_methodology);
            free(sosm->grid[r][c].fallback_methodology);
        }
    free(sosm);
}

void cst_sosm_classify(CSTSOSM* sosm) {
    int r = (int)sosm->sys->context.complexity;
    int c = (int)sosm->sys->context.relationship;
    sosm->best_row = r; sosm->best_col = c;
    sosm->grid[r][c].fit_score = 1.0;
    sosm->grid[r][c].case_count++;
}

void cst_sosm_find_best_cell(CSTSOSM* sosm) {
    double best = -1.0;
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++) {
            double fit = cst_sosm_fit(sosm, r, c);
            if (fit > best) { best = fit; sosm->best_row = r; sosm->best_col = c; }
        }
}

double cst_sosm_fit(const CSTSOSM* sosm, int row, int col) {
    double comp_dist = fabs((double)(row - (int)sosm->sys->context.complexity));
    double rel_dist = fabs((double)(col - (int)sosm->sys->context.relationship));
    return 1.0 / (1.0 + comp_dist + rel_dist * 1.5);
}

const char* cst_sosm_best_methodology(const CSTSOSM* sosm) {
    return sosm->grid[sosm->best_row][sosm->best_col].ideal_methodology;
}

bool cst_sosm_needs_critical(const CSTSOSM* sosm) {
    return sosm->best_col == (int)CST_COERCIVE ||
           (sosm->best_col == (int)CST_PLURALIST && sosm->best_row == (int)CST_COMPLEX);
}

bool cst_sosm_needs_soft(const CSTSOSM* sosm) {
    return sosm->best_col >= (int)CST_PLURALIST;
}

bool cst_sosm_is_coercive_cell(const CSTSOSM* sosm) { return sosm->best_col == (int)CST_COERCIVE; }
bool cst_sosm_is_pluralist_cell(const CSTSOSM* sosm) { return sosm->best_col == (int)CST_PLURALIST; }
bool cst_sosm_is_unitary_cell(const CSTSOSM* sosm) { return sosm->best_col == (int)CST_UNITARY; }
double cst_sosm_context_stability(const CSTSOSM* sosm) { return 1.0/(1.0+fabs(sosm->best_row-1.0)+fabs(sosm->best_col-1.0)); }
int cst_sosm_methodology_count(void) { return 9; }

void cst_sosm_track_context_shift(CSTSOSM* sosm, CSTComplexity nc, CSTRelationship nr) {
    int or_ = sosm->best_row, oc = sosm->best_col;
    sosm->sys->context.complexity = nc; sosm->sys->context.relationship = nr;
    cst_sosm_classify(sosm);
    if (or_ != (int)nc || oc != (int)nr) printf("SOSM shifted: (%d,%d)->(%d,%d)\n", or_, oc, (int)nc, (int)nr);
}

const char* cst_sosm_cell_methodology(int row, int col) {
    const char* g[3][3] = {{"Hard SE","SAST","CSH"},{"SD/VSM","SSM","TSI"},{"Crisis","Postmodern","Emancipatory"}};
    if (row < 0 || row > 2 || col < 0 || col > 2) return "Unknown";
    return g[row][col];
}

void cst_sosm_print(const CSTSOSM* sosm) {
    printf("=== SOSM: %s ===\n", sosm->sys->name);
    printf("Context: %s\n", cst_sosm_cell_label(sosm->sys));
    printf("Best Methodology: %s\n", cst_sosm_best_methodology(sosm));
    printf("Needs Critical: %s  Needs Soft: %s\n",
           cst_sosm_needs_critical(sosm) ? "YES" : "NO",
           cst_sosm_needs_soft(sosm) ? "YES" : "NO");
}

void cst_sosm_print_grid(const CSTSOSM* sosm) {
    printf("SOSM Grid:\n%-20s %-20s %-20s %-20s\n", "", "Unitary", "Pluralist", "Coercive");
    const char* row_labels[] = {"Simple", "Complex", "Chaotic"};
    for (int r = 0; r < 3; r++) {
        printf("%-20s", row_labels[r]);
        for (int c = 0; c < 3; c++) {
            char marker = (r == sosm->best_row && c == sosm->best_col) ? '*' : ' ';
            printf("%c%-19s", marker, sosm->grid[r][c].ideal_methodology);
        }
        printf("\n");
    }
}

double cst_sosm_cell_density(const CSTSOSM* sosm, int row, int col){return sosm->grid[row][col].case_count>0?1.0:0.0;}
int cst_sosm_total_cases(const CSTSOSM* sosm){int t=0;for(int r=0;r<3;r++)for(int c=0;c<3;c++)t+=sosm->grid[r][c].case_count;return t;}
double cst_sosm_dominant_quadrant(const CSTSOSM* sosm){int total=cst_sosm_total_cases(sosm);return(total>0)?(double)sosm->grid[sosm->best_row][sosm->best_col].case_count/total:0.0;}
double cst_sosm_complexity_sensitivity(const CSTSOSM* sosm){int sc=0;for(int c=0;c<3;c++)sc+=sosm->grid[sosm->best_row][c].case_count;int total=cst_sosm_total_cases(sosm);return(total>0)?(double)sc/total:0.0;}
double cst_sosm_relationship_sensitivity(const CSTSOSM* sosm){int sr=0;for(int r=0;r<3;r++)sr+=sosm->grid[r][sosm->best_col].case_count;int total=cst_sosm_total_cases(sosm);return(total>0)?(double)sr/total:0.0;}
bool cst_sosm_is_boundary_cell(const CSTSOSM* sosm){return sosm->best_row==0||sosm->best_row==2||sosm->best_col==0||sosm->best_col==2;}
bool cst_sosm_is_center_cell(const CSTSOSM* sosm){return sosm->best_row==1&&sosm->best_col==1;}
int cst_sosm_adjacent_cell_count(const CSTSOSM* sosm, int* rows, int* cols, int max_n){int n=0;int dr[]={-1,1,0,0},dc[]={0,0,-1,1};for(int i=0;i<4&&n<max_n;i++){int nr=sosm->best_row+dr[i],nc=sosm->best_col+dc[i];if(nr>=0&&nr<3&&nc>=0&&nc<3){rows[n]=nr;cols[n]=nc;n++;}}return n;}
