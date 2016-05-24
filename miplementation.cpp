#include <d3d9.h>
#include <d3dx9.h>
#include <wrl/client.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <shlwapi.h>

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "shlwapi.lib")

int main(int argc, const char* argv[])
{
    if (argc < 3)
    {
        printf("miplementation <texture> <texture>\n\ndirectly copies mip levels from texture with mips into texture of same format and dimensions but without mips\n");
        return 1;
    }

    HRESULT res;

    ComPtr<IDirect3D9> d3d9(Direct3DCreate9(D3D_SDK_VERSION));
    ComPtr<IDirect3DDevice9> device;
    ComPtr<IDirect3DTexture9> textures[2];
    ComPtr<IDirect3DSurface9> mip0_surfaces[2];
    D3DSURFACE_DESC mip0_descs[2];

    D3DPRESENT_PARAMETERS pp = {};
    res = d3d9->CreateDevice(0, D3DDEVTYPE_NULLREF, NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &device);
    if (!SUCCEEDED(res))
        goto cleanup;

    for (int i = 0; i < 2; ++i)
    {
        res = D3DXCreateTextureFromFileExA(device.Get(), argv[1 + i], D3DX_DEFAULT, D3DX_DEFAULT, D3DX_FROM_FILE, 0, D3DFMT_FROM_FILE, D3DPOOL_SYSTEMMEM, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &textures[i]);
        if (!SUCCEEDED(res))
            goto cleanup;
    }

    int src_index;
    if (textures[0]->GetLevelCount() == 1)
        src_index = 0;
    else if (textures[1]->GetLevelCount() == 1)
        src_index = 1;
    else
    {
        res = 2;
        goto cleanup;
    }

    int dest_index = !src_index;
    if (textures[dest_index]->GetLevelCount() < 2)
    {
        res = 3;
        goto cleanup;
    }

    for (int i = 0; i < 2; ++i)
    {
        res = textures[i]->GetLevelDesc(0, &mip0_descs[i]);
        if (!SUCCEEDED(res))
            goto cleanup;
    }

    if (memcmp(&mip0_descs[0], &mip0_descs[1], sizeof(mip0_descs[0])) != 0)
    {
        res = 4;
        goto cleanup;
    }

    for (int i = 0; i < 2; ++i)
    {
        res = textures[i]->GetSurfaceLevel(0, &mip0_surfaces[i]);
        if (!SUCCEEDED(res))
            goto cleanup;
    }

    res = D3DXLoadSurfaceFromSurface(mip0_surfaces[dest_index].Get(), NULL, NULL, mip0_surfaces[src_index].Get(), NULL, NULL, D3DX_FILTER_NONE, 0);
    if (!SUCCEEDED(res))
        goto cleanup;

    res = D3DXSaveTextureToFileA(argv[1 + src_index], D3DXIFF_DDS, textures[dest_index].Get(), NULL);
    if (!SUCCEEDED(res))
        goto cleanup;

cleanup:

    for (int i = 0; i < 2; ++i)
    {
        mip0_surfaces[i] = nullptr;
        textures[i] = nullptr;
    }

    device = nullptr;
    d3d9 = nullptr;
    return res;
}