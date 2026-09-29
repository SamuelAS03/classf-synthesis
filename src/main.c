#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <complex.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NLENGTH 2

/* ============================================================================= */
/* HMI FUNCTIONS */
/* ============================================================================= */

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

uint8_t readUint8(const char *prompt) {
    uint8_t val;
    int res;
    do {
        printf("%s", prompt);
        res = scanf("%hhu", &val);
        if (res != 1 || val == 0) {
            printf("[ERROR] Invalid insert! Enter a positive number.\n");
            clearInputBuffer();
        }
    } while (res != 1 || val == 0);
    clearInputBuffer();
    return val;
}

double readDouble(const char *prompt) {
    double val;
    int res;
    do {
        printf("%s", prompt);
        res = scanf("%lf", &val);
        if (res != 1) {
            printf("[ERROR] Invalid insert! Enter a real number.\n");
            clearInputBuffer();
        }
    } while (res != 1);
    clearInputBuffer();
    return val;
}

uint8_t menu() {
    printf("====================================================\n");
    printf("   CLASS F MATCHING CALCULATOR  \n");
    printf("====================================================\n");
    printf("1. Class F\n");
    printf("2. Class F-1\n");
    printf("9. Exit\n");

    uint8_t menuOpt = readUint8("Choose an option: ");

    return menuOpt;
}

double complex getZin_f0() {
    printf("Enter f1 impedance:\n");
    double r = 0.0, x = 0.0;
    while (1) {
        printf("R (Ohm) [R >= 0]: ");
        if (scanf("%lf", &r) == 1 && r >= 0.0) {
            clearInputBuffer();
            break;
        }
        printf("Invalid insert! Resistance R must be zero or a positive number.\n");
        clearInputBuffer();
    }

    while (1) {
        printf("X (Ohm) (+ inductive / - capacitive): ");
        if (scanf("%lf", &x) == 1) {
            clearInputBuffer();
            break;
        }
        printf("Invalid insert! Reactance X must be a real number.\n");
        clearInputBuffer();
    }

    return r + x * I;
}

void createZin(uint8_t nFreqs, double freq[nFreqs], uint8_t order[nFreqs], uint8_t option[nFreqs], double complex Zin[nFreqs], uint8_t menuOpt) {
    for (size_t i = 0; i < nFreqs; i++) {
        freq[i] = i + 1.0;
        order[i] = nFreqs - 1 - i;
    }

    Zin[0] = getZin_f0();

    while (1) {
        uint8_t val;
        printf("Switch f0 line-stub matching type? [y=1/n=0]: ");
        if (scanf("%hhu", &val) == 1 && (val == 0 || val == 1)) {
            option[order[0]] = val;
            clearInputBuffer();
            break;
        }
        printf("Invalid insert! Enter '0' or '1'.\n");
        clearInputBuffer();
    }

    // Create harmonics impedances
    if (menuOpt == 1) {
        for (size_t i = 1; i < nFreqs; i++) {
            if ((i + 1) % 2) {    // odd harmonic
                Zin[i] = 1e9;   // OC
            }
            else {              // even harmonic
                Zin[i] = 1e-9;  // SC
            }
        }
    }
    else {
        for (size_t i = 1; i < nFreqs; i++) {
            if ((i + 1) % 2) {    // odd harmonic
                Zin[i] = 1e-9;  // SC
            }
            else {              // evem harmonic
                Zin[i] = 1e9;   // OC
            }
        }
    }
}

void printZ(uint8_t nFreqs, double complex Zin[nFreqs]) {
    for (size_t i = 0; i < nFreqs; i++) {
        printf("| f%u ", i + 1);
        if (cimag(Zin[i]) < 0) {
            printf("| %.1f - i%.1f ", creal(Zin[i]), fabs(cimag(Zin[i])));
        }
        else {
            printf("| %.1f + i%.1f ", creal(Zin[i]), cimag(Zin[i]));
        }
        printf("|\n");
    }
}

/* ============================================================================= */
/* CALCULATOR FUNCTIONS */
/* ============================================================================= */

double deg2rad(double deg) {
    return deg * M_PI / 180;
}

double rad2deg(double rad) {
    return rad * 180 / M_PI;
}

double complex z2rho(double complex z, double Z0) {
    return (z - Z0) / (z + Z0);
}

double stubCalc(double l_linha, double complex z, double Z0, double complex ZL) {
    double Y0 = 1.0 / Z0;
    double complex YL = 1.0 / ZL;

    // Z at node after the end of the line 
    z = Z0 * (z + I * Z0 * tan(l_linha)) / (Z0 + I * z * tan(l_linha));

    /* UNCOMMENT TO DEBUG */
    /*
    printf("Z_no: %.3f + i%.3f\n", creal(z), cimag(z));
    */

    double complex y = 1.0 / z;
    double B_node = cimag(y);
    double B_stub = cimag(YL) - B_node;

    /* UNCOMMENT TO DEBUG */
    /*
    printf("Y node: %.4f + i%.4f (Re{Y} must be equal to 0.02)\n", creal(y), B_node);
    printf("B that must be obtained from stub: i%.4f\n", B_stub);
    */

    // Stub OC -> B_stub = -B_node
    double l_stub = atan(B_stub / Y0);

    while (l_stub < 0) l_stub += M_PI;

    return l_stub;
}

double lineCalc(double complex z, double Z0, double complex ZL, uint8_t option) {
    double Y0 = 1.0 / Z0;
    double G = creal(1.0 / ZL);

    double complex rho_in = z2rho(z, Z0);
    double phaseZin = carg(rho_in);
    double magZin = cabs(rho_in);

    /* UNCOMMENT TO DEBUG */
    /*
    printf("rho_in: %.3f + i%.3f, mag: %.3f, phase: %.3f rad\n", 
           creal(rho_in), cimag(rho_in), magZin, phaseZin);
    */

    double l_linha = 0;
    double arg_acos;

    if (magZin != 0) {
        if (fabs(G - Y0) < 1e-9) {
            arg_acos = -magZin;
        }

        else {
            arg_acos = (1.0 / (2.0 * magZin)) * ((Y0 / G - 1.0) - pow(magZin, 2) * (Y0 / G + 1.0));
        }

        if (option) {
            l_linha = (phaseZin - acos(arg_acos)) / 2.0;
        }
        else {
            l_linha = (phaseZin + acos(arg_acos)) / 2.0;
        }
        
        
        while (l_linha < -0.0001) l_linha += M_PI;
        if (l_linha >= -0.0001 && l_linha < 0) l_linha = 1e-9;
    }

    return l_linha;    
}

void electricalLengthCalc(double l[], double complex z, double Z0, double complex ZL, double fnorm, uint8_t option) {
    l[0] = lineCalc(z, Z0, ZL, option);
    l[1] = stubCalc(l[0], z, Z0, ZL);

    /* UNCOMMENT TO DEBUG */
    /*
    printf("\n\n\nLength: %.2f, %.2f, fnorm: %.2f \n\n\n", l[0], l[1], fnorm);
    */

    l[0] *= fnorm;
    l[1] *= fnorm;

    /* UNCOMMENT TO DEBUG */
    /*
    printf("linha: %.2f (deg), stub: %.2f (deg)\n", rad2deg(l[0]), rad2deg(l[1]));
    */
}

double complex getZin(double complex z, double Z0, double l) {
    // Formula: impedance variation as a function of line length
    double complex Z_node = 0;

    if (fabs(l - (M_PI / 2)) < 0.05) {
        if (fabs(z) < 1) {
            Z_node = 1e9 + 0 * I;
            return Z_node;
        }

        Z_node = pow(Z0, 2) / z;
    }
    else {
        Z_node = Z0 * (z + I * Z0 * tan(l)) / (Z0 + I * z * tan(l));
    }

    return Z_node;
}

double complex getZStubOC(double Z0, double l) {
    // Formula: Impedance seen into a OC-terminated stub for a given length line
    double complex Z_stub = - I * Z0 * 1 / tan(l);
    
    return Z_stub;
}

double complex getUnparallel(double complex Z1, double complex Zparallel) {
    // Formula: parallel inverse of 2 impedances
    double complex Z2; // = (Z1 * Zparallel) / (Z1 - Zparallel);

    Z2 = 1 / Zparallel - 1 / Z1;
    Z2 = 1 / Z2;

    return Z2;
}

double complex getParallel(double complex Z1, double complex Z2) {
    // Formula: parallel of 2 impeadances
    double complex Zparallel = (Z1 * Z2) / (Z1 + Z2);

    return Zparallel;
}

void getNextZNode(uint8_t node, uint8_t nFreqs, double complex Znodes[nFreqs][nFreqs + 1], double Z0, double l[NLENGTH], double fnorm[nFreqs]) {
    double complex *Zaux = calloc(nFreqs, sizeof(double complex));
    double complex *Zstub = calloc(nFreqs, sizeof(double complex));

    for (size_t i = 0; i < nFreqs; i++) {
        Zaux[i] = getZin(Znodes[i][node - 1], Z0, fnorm[i] * l[0]);
        Zstub[i] = getZStubOC(Z0, fnorm[i] * l[1]);
        Znodes[i][node] = getParallel(Zstub[i], Zaux[i]);
    }

    free(Zaux);
    free(Zstub);
}

void printZNodes(uint8_t nFreqs, complex Znodes[nFreqs][nFreqs + 1]) {
    printf("\nZ Nodes\n");
    printf("---------------------------------------------------------------------\n");
    for (size_t i = 0; i < nFreqs; i++) {
        printf("| f%u ", i);
        for (size_t j = 0; j < nFreqs + 1; j++) {
            if (cimag(Znodes[i][j]) < 0) {
                printf("| %.1f - i%.1f ", creal(Znodes[i][j]), fabs(cimag(Znodes[i][j])));
            }
            else {
                printf("| %.1f + i%.1f ", creal(Znodes[i][j]), cimag(Znodes[i][j]));
            }
        }
        printf("|\n");
    }
    printf("-----------------------------------------------------------------------\n");
}

void printLMatrix(uint8_t nFreqs, double l_matrix[nFreqs][NLENGTH]) {
    printf("\nLine and Stub matrix (deg)\n");
    printf("---------------------------------------------------------------------\n");
    for (size_t i = 0; i < nFreqs; i++) {
        printf("| pair %u ", i);
        for (size_t j = 0; j < NLENGTH; j++) {
            printf("| %.2f ", rad2deg(l_matrix[i][j]));
        }
        printf("|\n");
    }
    printf("-----------------------------------------------------------------------\n");
}

void calcCompleteNetwork(uint8_t nFreqs, double complex Znodes[nFreqs][nFreqs + 1], double l_matrix[nFreqs][NLENGTH], double Z0, double complex ZL, double freq[], uint8_t order[], uint8_t option[]) {
    double l[NLENGTH] = {}; 
    double complex *Zaux = calloc(nFreqs, sizeof(double complex));
    double complex *Zstub = calloc(nFreqs, sizeof(double complex));
    double complex *Znode = calloc(nFreqs, sizeof(double complex));
    
    /* INITIALIZING */
    double *fnorm = calloc(nFreqs, sizeof(double));
    for (size_t i = 0; i < nFreqs; i++) {
        fnorm[i] = freq[0] / freq[i];
    }

    for (size_t i = 0; i < nFreqs; i++) {
        Znode[i] = Znodes[i][0];
    }

    /* CALCULATE */
    uint8_t ord = 0;
    for (size_t i = 0; i < nFreqs; i++) {
        ord = order[i];

        electricalLengthCalc(l, Znode[ord], Z0, ZL, fnorm[ord], option[i]);
        l_matrix[i][0] = l[0];
        l_matrix[i][1] = l[1];

        for (size_t j = 0; j < nFreqs; j++) {
            Zaux[j] = getZin(Znode[j], Z0, l[0] / fnorm[j]);
            Zstub[j] = getZStubOC(Z0, l[1] / fnorm[j]);
            Znode[j] = getParallel(Zstub[j], Zaux[j]);

            /* UNCOMMENT TO DEBUG */
            /*
            printf("| f%u | line: %.2f + i%.2f ", j, creal(Zaux[j]), cimag(Zaux[j]));
            printf("| stub: %.2f + i%.2f ", creal(Zstub[j]), cimag(Zstub[j]));
            printf("| Znode: %.2f + i%.2f \n", creal(Znode[j]), cimag(Znode[j]));
            */
            
            Znodes[j][ord + 1] = Znode[j];
        }
        /* UNCOMMENT TO DEBUG */
        /*
        printZNodes(nFreqs, Znodes);
        */
    }
    /* UNCOMMENT TO DEBUG */
    /*
    printLMatrix(nFreqs, l_matrix);
    */

    free(Zaux);
    free(Zstub);
    free(Znode);
    free(fnorm);
}

void calcCompleteNetworkInv(uint8_t nFreqs, double complex ZnodesInv[nFreqs][nFreqs + 1], double l_matrix[nFreqs][NLENGTH], double Z0, double complex ZL, double freq[]) {
    double l[NLENGTH] = {};

    double complex *fnorm = calloc(nFreqs, sizeof(double complex));
    for (size_t i = 0; i < nFreqs; i++) {
        fnorm[i] = freq[0] / freq[i];
    }

    double complex *Zaux = calloc(nFreqs, sizeof(double complex));
    double complex *Zstub = calloc(nFreqs, sizeof(double complex));
    double complex *Znode = calloc(nFreqs, sizeof(double complex));
    uint8_t ord = 0;

    for (size_t i = 0; i < nFreqs; i++) {
        Znode[i] = ZL;
        // TODO: GET VERIFICATION
        ZnodesInv[i][nFreqs] = ZL;
    }
    
    for (int n = nFreqs - 1; n >= 0; n--) {
        l[0] = l_matrix[n][0];
        l[1] = l_matrix[n][1];
        for (int i = nFreqs - 1; i >= 0; i--) {
            Zstub[i] = getZStubOC(Z0, l[1] / fnorm[i]);
            Zaux[i] = getParallel(Zstub[i], Znode[i]);
            Znode[i] = getZin(Zaux[i], Z0, l[0] / fnorm[i]);

            /* UNCOMMENT TO DEBUG */
            /*
            printf("| f%u | stub: %.2f + i%.2f ", i, creal(Zstub[i]), cimag(Zstub[i]));
            printf("| parallel: %.2f + i%.2f ", creal(Zaux[i]), cimag(Zaux[i]) );
            printf("| Znextnode: %.2f + i%.2f \n", creal(Znode[i]), cimag(Znode[i]));
            */

            ZnodesInv[i][n] = Znode[i];
        }
    }

    /* UNCOMMENT TO DEBUG */
    /*
    printZNodes(nFreqs, ZnodesInv);
    */

    free(Zaux);
    free(Zstub);
    free(Znode);
    free(fnorm);
}

uint8_t initCalc(double complex Zin[], uint8_t nFreqs, double complex Znodes[nFreqs][nFreqs + 1]) {
    for (size_t i = 0; i < nFreqs; i++) {
        if (creal(Zin[i]) > 1e9) Zin[i] = 1e9 + cimag(Zin[i]) * I;
        else if (fabs(creal(Zin[i])) < 1e-9) Zin[i] = 1e-9 + cimag(Zin[i]) * I; 
        else if (creal(Zin[i]) < 0) return 1;

        if (cimag(Zin[i]) > 1e9) Zin[i] = creal(Zin[i]) + 1e9 * I;
        else if (fabs(cimag(Zin[i])) < 1e-9) Zin[i] = creal(Zin[i]) + 1e-9 * I; 
    }

    for (size_t i = 0; i < nFreqs; i++) {
        Zin[i] = conj(Zin[i]);
    }
    
    for (size_t i = 0; i < nFreqs; i++) {
        Znodes[i][0] = Zin[i];
    }

    /* UNCOMMENT TO DEBUG */
    /*
    printZNodes(nFreqs, Znodes);
    */

    return 0;
}

void printFinalResults(uint8_t nFreqs, complex ZnodesInv[nFreqs][nFreqs + 1], double l_matrix[nFreqs][NLENGTH], double freq[], double complex Zin[]) {
    char z_str[24];
    char z_target_str[24];

    printf("\nFinal Results\n");
    printf("+--------+----------+----------++--------+------------------------+------------------------+\n");
    printf("| Pair   | Line (\xF8) | Stub (\xF8) || freq   | Calculated Zin (Ohm)   | Target Zin (Ohm)       |\n");
    printf("+--------+----------+----------++--------+------------------------+------------------------+\n");

    for (size_t i = 0; i < nFreqs; i++) {
        double real_part = creal(ZnodesInv[i][0]);
        double imag_part = cimag(ZnodesInv[i][0]);
        char sign = (imag_part >= 0) ? '+' : '-';

        if (fabs(real_part) >= 1e5 || fabs(imag_part) >= 1e5) {
            snprintf(z_str, sizeof(z_str), "%.2e %c i%.2e", real_part, sign, fabs(imag_part));
        } else {
            snprintf(z_str, sizeof(z_str), "%.2f %c i%.2f", real_part, sign, fabs(imag_part));
        }

        double real_target = creal(Zin[i]);
        double imag_target = cimag(Zin[i]);
        // ATENTION: Zin is in conj form
        char sign_target = (imag_target >= 0) ? '-' : '+';

        if (fabs(real_target) >= 1e5 || fabs(imag_target) >= 1e5) {
            snprintf(z_target_str, sizeof(z_target_str), "%.2e %c i%.2e", real_target, sign_target, fabs(imag_target));
        } else {
            snprintf(z_target_str, sizeof(z_target_str), "%.2f %c i%.2f", real_target, sign_target, fabs(imag_target));
        }

        printf("| Pair %-2zu| %8.2f | %8.2f || f%-5u | %-22s | %-22s |\n", 
               i + 1, 
               rad2deg(l_matrix[i][0]), 
               rad2deg(l_matrix[i][1]),
               (unsigned int)(i + 1),
               z_str,
               z_target_str);
    }
    printf("+--------+----------+----------++--------+------------------------+------------------------+\n");
}

int main() {
    uint8_t menuOpt = 1;
    uint8_t classF = 1;
    uint8_t nFreqs;

    double Z0 = 50.0;
    double complex ZL = Z0;
    double complex z;

    double l[NLENGTH] = {};

while (menuOpt != 9) {
    menuOpt = menu();

    if (menuOpt == 9) {
        printf("\nExiting program...\n");
        return 0;
    }
    else if (menuOpt == 1 || menuOpt == 2) {
        do {
            nFreqs = readUint8("Enter the number of harmonics (1-10): ");
        } while (nFreqs > 10);

        double (*l_matrix)[NLENGTH] = calloc(nFreqs, sizeof(*l_matrix));
        double complex *Zin = calloc(nFreqs, sizeof(double complex));
        uint8_t *order = calloc(nFreqs, sizeof(uint8_t));
        double *freq = calloc(nFreqs, sizeof(double));
        double complex (*Znodes)[nFreqs + 1] = calloc(nFreqs, sizeof(*Znodes));
        double complex (*ZnodesInv)[nFreqs + 1] = calloc(nFreqs, sizeof(*ZnodesInv));
        uint8_t *option = calloc(nFreqs, sizeof(uint8_t));

        if (!l_matrix || !Zin || !order || !freq || !option || !Znodes || !ZnodesInv) {
            printf("Fatal Error! Program failed allocating memory!\n");
            return 1;
        }

        createZin(nFreqs, freq, order, option, Zin, menuOpt);

        /* UNCOMMENT TO DEBUG */
        /*
        printf("\n====================================================\n");
        printf("Target Impedances\n");
        printf("====================================================\n");
        printZ(nFreqs, Zin);
        printf("====================================================\n");
        printf("Calculating...\n");
        printf("====================================================\n");
        */
        
        // Calculate Complete Matching Network
        uint8_t res = initCalc(Zin, nFreqs, Znodes);
        if (res) {
            printf("\n[ERROR] Impedance is invalid!\n");
            free(l_matrix); free(Zin); free(order); free(freq); free(Znodes); free(ZnodesInv); free(option);
            return 0;
        }
        calcCompleteNetwork(nFreqs, Znodes, l_matrix, Z0, ZL, freq, order, option);

        // Calculate in Reverse for Retification
        calcCompleteNetworkInv(nFreqs, ZnodesInv, l_matrix, Z0, ZL, freq);
        printFinalResults(nFreqs, ZnodesInv, l_matrix, freq, Zin);

        free(l_matrix);
        free(Zin);
        free(order);
        free(freq);
        free(Znodes);
        free(ZnodesInv);
        free(option);
    }
    else {
        printf("[ERROR] Invalid insert. Enter a valid number.\n");
    }
}

    return 0;
}