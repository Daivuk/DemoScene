#pragma once
#include <d3d11.h>

struct sTexture
{
    ID3D11ShaderResourceView* view;
    int w;
    int h;
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

extern sTexture* res_textures;
extern sMesh* res_meshes;
extern sModel* res_models;
extern sCamera* res_cameras;

extern int res_textureCount;
extern int res_meshCount;
extern int res_modelCount;
extern int res_cameraCount;

extern sCamera* res_currentCamera;

void res_load();
