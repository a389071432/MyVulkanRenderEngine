#version 450

layout(location=0) in vec3 fragPos;
layout(location=1) in vec3 fragColor;
layout(location=2) in vec3 normal;

layout(location=0) out vec4 outColor;


layout(set=0, binding=0) uniform sampler texIrradiance;
layout(set=0, binding=1) uniform sampler texVisibility;

layout(push_constant) uniform PushConstants{
int n_dir;
int n_probe_x;     
int n_probe_y; 
int n_probe_z; 
float grid_size;

}pushConst;

const vec3 ambient = vec3(0.15, 0, 0);

ivec3 getProbeBaseCoord(vec3 pos){
vec3 ipos = pos/pushConst.grid_size;
int x = clamp(ipos.x, 0, pushConst.n_probe_x-2);
int y = clamp(ipos.y, 0, pushConst.n_probe_y-2);
int z = clamp(ipos.z, 0, pushConst.n_probe_z-2);
return ivec3(x,y,z);
}

vec2 oct_encode(vec3 v){
float sum_abs = abs(v.x)+abs(v.y)+abs(v.z);
vec3 p = v/sum_abs;
vec2 oct_coord = p.xy;
if(p.z<0){
float x = sign(oct_coord.x)*(1-abs(oct_coord.y));
float y = sign(oct_coord.y)*(1-abs(oct_coord.x));
oct_coord.x = x;
oct_coord.y = y;
}
return 2*oct_coord + vec2(0.5,0.5);
}

// get UV coord to sample from the probe-direction-irradiancce texture
vec2 getUV(vec3 normal, ivec3 probe_coord){
int tex_width = pushConst.n_probe_x * pushConst.n_probe_x * pushConst.n_dir;
int tex_height = pushConst.n_probe_z * pushConst.n_dir;
int base_coord_x = probe_coord.y * pushConst.n_probe_x + probe_coord.x;
int base_coord_y = probe_coord.z;
vec2 base_coord = vec2(base_coord_x,base_coord_y)*pushConst.n_dir;
base_coord.x /= tex_width;
base_coord.y /= tex_height;
vec2 offset = oct_encode(normal);
vec2 UV = base_coord + offset;
return UV;
}

// get the trilinear interpolation weight of a reference point of the corner of the cube
float getTrilinearWeight(vec3 ref, vec3 target, float unit_size){
vec3 diff = ref - target;
diff /= unit_size;
float weight = (1-abs(diff.x))*(1-abs(diff.y))*(1-abs(diff.z));
return weight;
}

const int DIFF[8][3] = {
{0,0,0},
{1,0,0},
{1,1,0},
{0,1,0},
{0,0,1},
{1,0,1},
{1,1,1},
{0,1,1}
};

void main(){
// apply direct lighting
// currently we just use the ambient 
// TODO: phong lighting and PBR materials
vec3 color(0);
color += ambient;

// get the base probe which lies at the corner of the grid that contains the fragment
ivec3 base_probe_coord = getProbeBaseCoord(fragPos);

// sample the irradiance from the 8 neighbouring probes (8 corners of the grid)
// assign weight to each probe using trilinear interpolation and chebyshev visibility testing
// use the probe-frag direction to sample the probe-direction-irradiancce texture
// order: from lower z to higher z, counter-clockwise per z
float total_weight = 0;
vec3 irradiance(0); 
for(int p=0;p<8;p++){
ivec3 probe_coord = base_probe_coord + ivec3(DIFF[p][0],DIFF[p][1],DIFF[p][2]);
vec3 probe_pos = probe_coord*pushConst.grid_size;
vec3 dir_probe_frag = fragPos - probe_pos;
float dis_probe_frag = sqrt(dir_probe_frag*dir_probe_frag);
vec2 UV = getUV(normal, probe_coord);
vec3 ird = texture(texIrradiance, UV).rgb;
vec2 vis = texture(texVisibility, UV).rg;
float mean_dis = vis.x;
float sqr_dis = vis.y;
float variance = mean_dis*mean_dis - sqr_dis;
float weight = 1.0;
float cheby_weight = 1.0;
if(dis_probe_frag>mean_dis){
float diff = dis_probe_frag - mean_dis;
cheby_weight = variance/(variance+diff*diff);
} 
weight *= cheby_weight;

float tri_weight= getTrilinearWeight(probe_pos, fragPos, pushConst.grid_size);
weight *= tri_weight;

irradiance += weight*ird;
total_weight += weight;
}

vec3 indirectColor = irradiance/total_weight;
color += indirectColor;

outColor = vec4(color,1.0);
}