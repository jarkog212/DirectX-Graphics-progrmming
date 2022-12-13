#define MAX_ALTITUDE 50.f                                       //landscape max altitude
#define NUM_OF_LIGHTS 2                                         //max 8 per shader
#define MAX_SHADOW_PASSES 64                                    //used for shadow blurring
#define ENUM_IF(input, compare) abs(input - compare) < 0.0001   //safe float to int comparisons