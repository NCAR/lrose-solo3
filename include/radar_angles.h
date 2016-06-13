#ifndef radar_angles_hh
#define radar_angles_hh

struct radar_angles {
    /* all angles are in radians
     */
    float azimuth;
    float elevation;
    float x;
    float y;
    float z;
    float psi;
    float rotation_angle;
    float tilt;
};
#endif
