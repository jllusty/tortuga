#ifndef SAMPLER_HPP

#include <vector>
#include <memory>
#include <utility>
#include <iostream>

static float max(float a, float b) {
    return (a<b)?b:a;
}
static float min(float a, float b) {
    return (a<b)?a:b;
}

#include "vec3.hpp"

namespace sampler {
    using vector1f = std::vector<float>;
    using vector2f = std::vector<vector1f>;
    using vector3f = std::vector<vector2f>;
    vector3f img;
    // image dimensions
    int width = 0, height = 0, depth = 0;
    // world coordinates
    float wX0 = 0, wXF = 0;
    float wY0 = 0, wYF = 0;
    float wZ0 = 0, wZF = 0;
    // set rasterization context
    void setContext(int imageWidth, int imageHeight, int imageDepth,
        float worldX0, float worldXF, 
        float worldY0, float worldYF, 
        float worldZ0, float worldZF)
    {
        width = imageWidth; height = imageHeight, depth = imageDepth;
        wX0 = worldX0; wXF = worldXF;
        wY0 = worldY0; wYF = worldYF;
        wZ0 = worldZ0; wZF = worldZF;
        img = vector3f(width,vector2f(height,vector1f(depth,0)));
    }
    void sampleLine(vec3 r0, vec3 rf) {
        // transform world -> voxel
        rf = (rf-vec3(wX0,wY0,wZ0))/vec3(wXF-wX0,wYF-wY0,wZF-wZ0)*vec3(width,height,depth);
        r0 = (r0-vec3(wX0,wY0,wZ0))/vec3(wXF-wX0,wYF-wY0,wZF-wZ0)*vec3(width,height,depth);
        float dx = rf.x()-r0.x(), dy = rf.y()-r0.y(), dz = rf.z()-r0.z();
        //std::cout << "(sampler): dr=" << "(" << dx << "," << dy << "," << dz << ")\n";
        float step = max(abs(dx),max(abs(dy),abs(dz)))*4.0;
        dx = dx/step; dy = dy/step, dz = dz/step;
        float x = r0.x(), y = r0.y(), z = r0.z();
        for(int c = 1; c <= step; ++c) {
            int i = x;//((x-wX0)/(wXF-wX0)*(float)width);
            int j = y;//((y-wY0)/(wYF-wY0)*(float)height);
            int k = z;//((z-wZ0)/(wZF-wZ0)*(float)depth);
            if((i<0)||(i>width-1)) break;
            if((j<0)||(j>height-1)) break;
            if((k<0)||(k>depth-1)) break;
            img[i][j][k] = 1.0f;
            //std::cout << "(sampler): " << "[" << i << "," << j << "," << k << "]\n";
            x += dx; y += dy; z += dz;
        }
    }
    void sampleSphere()
    {
        for(int i = 0; i < img.size(); ++i) {
        for(int j = 0; j < img[0].size(); ++j) {
        for(int k = 0; k < img[0][0].size(); ++k) {
            // transform voxel -> world space
            float x = (float)i/sampler::width*(sampler::wXF-sampler::wX0)+sampler::wX0;
            float y = (float)j/sampler::height*(sampler::wYF-sampler::wY0)+sampler::wY0;
            float z = (float)k/sampler::depth*(sampler::wZF-sampler::wZ0)+sampler::wZ0;
            vec3 p(x,y,z);
            if(sqrt(dot(p,p)) < sampler::width) img[i][j][k] = 1.0;
        }}}
    }
    void sampleLines(std::vector<std::pair<vec3,vec3>> lines)
    {
        for(int i = 0; i < img.size(); ++i) {
        for(int j = 0; j < img[0].size(); ++j) {
        for(int k = 0; k < img[0][0].size(); ++k) {
            // transform voxel -> world space
            float x = (float)i/sampler::width*(sampler::wXF-sampler::wX0)+sampler::wX0;
            float y = (float)j/sampler::height*(sampler::wYF-sampler::wY0)+sampler::wY0;
            float z = (float)k/sampler::depth*(sampler::wZF-sampler::wZ0)+sampler::wZ0;
            vec3 p(x,y,z);
            // get distance to line segment - if less than d, draw that voxel
            for(int li = 0; li < lines.size(); ++li) {
                // get line distance
                vec3 rf = lines[li].second; vec3 r0 = lines[li].first;
                float ld = sqrt(dot(rf-r0,rf-r0));
                float tH = dot(p-r0,rf-r0)/dot(rf-r0,rf-r0);
                if((tH<0)||(tH>1)) continue;
                vec3 pL = r0 + tH*(rf-r0);
                float d = sqrt(dot(pL-p,pL-p));
                //if(d <= ld/4.0f*(0.5+0.1*tH)) {
                if(d <= 1.0f) {
                    img[i][j][k] += 1.0f;
                }
            }
        }}}
    }
    void sampleLinesC(std::vector<std::pair<vec3,vec3>> lines)
    {
        for(int i = 0; i < img.size(); ++i) {
        for(int j = 0; j < img[0].size(); ++j) {
        for(int k = 0; k < img[0][0].size(); ++k) {
            // transform voxel -> world space
            float x = (float)i/sampler::width*(sampler::wXF-sampler::wX0)+sampler::wX0;
            float y = (float)j/sampler::height*(sampler::wYF-sampler::wY0)+sampler::wY0;
            float z = (float)k/sampler::depth*(sampler::wZF-sampler::wZ0)+sampler::wZ0;
            vec3 p(x,y,z);
            // get distance to line segment - if less than d, draw that voxel
            for(int li = 0; li < lines.size(); ++li) {
                // get line distance
                vec3 rf = lines[li].second; vec3 r0 = lines[li].first;
                float ld = sqrt(dot(rf-r0,rf-r0));
                float tH = dot(p-r0,rf-r0)/dot(rf-r0,rf-r0);
                if((tH<0)||(tH>1)) continue;
                vec3 pL = r0 + tH*(rf-r0);
                float d = sqrt(dot(pL-p,pL-p));
                if(d <= 1.0f) {
                    img[i][j][k] += 1.0f;
                }
            }
        }}}
    }
}

#define SAMPLER_HPP
#endif