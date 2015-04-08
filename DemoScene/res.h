#pragma once
#include <d3d11.h>
#include <cinttypes>

#define RES_DIFFUSE     0
#define RES_NORMAL      1
#define RES_MATERIAL    2

struct sTexture
{
    ID3D11ShaderResourceView* view[3];
    int w;
    int h;
    uint32_t* data[3];
};

struct sMesh
{
    sTexture* texture;
    sTexture* normalMap;
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    UINT indexCount;
};

struct sModel
{
    int count;
    sMesh **meshes;
    float *transform;
};

struct sCamera
{
    float *pos;
    float *lookAt;
    float *transform;
};

extern sTexture*    res_textures;
extern sMesh*       res_meshes;
extern sModel*      res_models;
extern sCamera*     res_cameras;
extern uint8_t*     res_palette;

extern int res_textureCount;
extern int res_meshCount;
extern int res_modelCount;
extern int res_cameraCount;

extern sCamera* res_currentCamera;

void res_load();
