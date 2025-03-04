#include "Scene.h"

#include "Shader.h"


Scene::Scene():
    scene_width { FRAME_BUFFER_WIDTH }, 
    scene_height { FRAME_BUFFER_HEIGHT },
    camera { nullptr }
{

}

Scene::~Scene() {
    
}


bool Scene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
    return false;
}

bool Scene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
    return false;
}

bool Scene::ProcessInput(UCHAR* pKeysBuffer) {
    return false;
}


ID3D12RootSignature* Scene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice) {
    ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

    D3D12_ROOT_PARAMETER pd3dRootParameters[3];

    pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
    pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
    pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    pd3dRootParameters[1].Constants.Num32BitValues = 32;
    pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
    pd3dRootParameters[1].Constants.RegisterSpace = 0;
    pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
    pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
    pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


    D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
    ::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));

    d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
    d3dRootSignatureDesc.pParameters = pd3dRootParameters;
    d3dRootSignatureDesc.NumStaticSamplers = 0;
    d3dRootSignatureDesc.pStaticSamplers = NULL;
    d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;


    ID3DBlob* pd3dSignatureBlob = NULL;
    ID3DBlob* pd3dErrorBlob = NULL;
    ::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        &pd3dSignatureBlob, &pd3dErrorBlob);

    pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(),
        pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), 
        (void**)&pd3dGraphicsRootSignature);

    if(pd3dSignatureBlob) pd3dSignatureBlob->Release();
    if(pd3dErrorBlob) pd3dErrorBlob->Release();

    return pd3dGraphicsRootSignature;
}

ID3D12RootSignature* Scene::GetGraphicsRootSignature() {
    return m_pd3dGraphicsRootSignature;
}


void Scene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {
    UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
    m_pd3dcbLights = ::CreateBufferResource(
        pd3dDevice, pd3dCommandList, 
        nullptr, ncbElementBytes, 
        D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, 
        nullptr
    );

    m_pd3dcbLights->Map(0, nullptr, (void**)&m_pcbMappedLights);
}

void Scene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) {
    ::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
    ::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
    ::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
}

void Scene::ReleaseShaderVariables() {
    if(m_pd3dcbLights) {
        m_pd3dcbLights->Unmap(0, NULL);
        m_pd3dcbLights->Release();
    }
}


void Scene::ReleaseUploadBuffers() {
    for(CGameObject* obj : m_pObjects) {
        obj->ReleaseUploadBuffers();
    }
}


void Scene::BuildDefaultLightsAndMaterials() {

}

void Scene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {
    m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
}

void Scene::ReleaseObjects() {
    if(m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

    for(CGameObject* obj : m_pObjects) {
        obj->Release();
    }

    ReleaseShaderVariables();

    if(m_pLights) delete[] m_pLights;
}


void Scene::AnimateObjects(float fTimeElapsed) {
    m_fElapsedTime = fTimeElapsed;

    for(CGameObject* obj : m_pObjects) {
        obj->Animate(fTimeElapsed, NULL);
    }
}



void Scene::Render(ID3D12GraphicsCommandList* pd3dCommandList) {
    pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

    camera->SetViewportsAndScissorRects(pd3dCommandList);
    camera->UpdateShaderVariables(pd3dCommandList);

    UpdateShaderVariables(pd3dCommandList);

    D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
    pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights

    //씬을 렌더링하는 것은 씬을 구성하는 게임 객체(셰이더를 포함하는 객체)들을 렌더링하는 것이다.
    for(CGameObject* obj : m_pObjects) {
        obj->Animate(m_fElapsedTime, NULL);
        obj->UpdateTransform(NULL);
        obj->Render(pd3dCommandList, camera);
    }
}


void Scene::setSceneSize(int width, int height) {
    scene_width = width;
    scene_height = height;

    this->adaptCamera();
}

int Scene::getSceneWidth() {
    return scene_width;
}

int Scene::getSceneHeight() {
    return scene_height;
}


void Scene::adaptCamera() {
    camera->SetViewport(0, 0, scene_width, scene_height, 0.0f, 1.0f);
    camera->SetScissorRect(0, 0, scene_width, scene_height);
    camera->GenerateProjectionMatrix(
        near_plane_distance, far_plane_distance,
        float(scene_width) / float(scene_height),
        fov
    );
}



















////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////





















GameScene::GameScene(): Scene { } {
    bg_color = { 0.0f, 0.125f, 0.3f, 1.0f };
    near_plane_distance = 0.1f;
    far_plane_distance = 500.0f;
}

GameScene::~GameScene() {

}


void GameScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {
    Scene::CreateShaderVariables(pd3dDevice, pd3dCommandList);

    camera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}


void GameScene::BuildDefaultLightsAndMaterials() {
    const int enemy_light_count = 10;
    enemy_lights.reserve(enemy_light_count);

    m_nLights = 1 + 1 + enemy_light_count;        // 플레이어, 태양, 적기 
    m_pLights = new LIGHT[m_nLights];
    ::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

    m_xmf4GlobalAmbient = XMFLOAT4 { 0.2f, 0.2f, 0.2f, 1.0f };

    m_pLights[0].m_bEnable = true;
    m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
    m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.6f, 1.0f);
    m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_pLights[0].m_xmf4Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 0.0f);
    m_pLights[0].m_xmf3Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);

    m_pLights[1].m_bEnable = true;
    m_pLights[1].m_nType = SPOT_LIGHT;
    m_pLights[1].m_fRange = 500.0f;
    m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
    m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
    m_pLights[1].m_fFalloff = 8.0f;
    m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(20.0f));
    m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(10.0f));

    player_searchlight = &m_pLights[1];

    for(int i=2; i<m_nLights; ++i) {
        m_pLights[i].m_bEnable = true;
        m_pLights[i].m_nType = SPOT_LIGHT;
        m_pLights[i].m_fRange = 500.0f;
        m_pLights[i].m_xmf4Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        m_pLights[i].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.0f, 0.0f, 1.0f);
        m_pLights[i].m_xmf4Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.0f);
        m_pLights[i].m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
        m_pLights[i].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
        m_pLights[i].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
        m_pLights[i].m_fFalloff = 8.0f;
        m_pLights[i].m_fPhi = (float)cos(XMConvertToRadians(16.0f));
        m_pLights[i].m_fTheta = (float)cos(XMConvertToRadians(8.0f));
        
        enemy_lights.push_back(&m_pLights[i]);
    }
}

void GameScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {
    Scene::BuildObjects(pd3dDevice, pd3dCommandList);

    CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

    BuildDefaultLightsAndMaterials();

    {
        camera = new TankCamera { };
        adaptCamera();
    }

    CMeshLoadInfo bullet_info = CMeshLoadInfo::CubeInfo(0.6f, 0.6f, 3.0f);
    CMesh* bullet_mesh = new CMeshIlluminatedFromFile { pd3dDevice, pd3dCommandList, &bullet_info };

    {
        m_pPlayer = new CTankPlayer {
            pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature
        };
        m_pPlayer->SetPosition(XMFLOAT3 { 0.0f, 0.0f, 0.0f });
        m_pPlayer->setBoundingBox({ 0.0f, 0.5f, 0.0f }, { 0.5f, 0.5f, 1.0f });

        m_pPlayer->setBulletMesh(bullet_mesh);
        m_pPlayer->setBulletBoundingBox({ 0.0f, 0.0f, 0.0f }, { 0.3f, 0.3f, 2.0f });        // 너무 빨라서 안맞을수 있으니 길이를 늘려서 충돌확률 높이기

        m_pObjects.push_back(m_pPlayer);
    }

    {
        CGameObject* helli_model = CGameObject::LoadGeometryFromFile(
            pd3dDevice, pd3dCommandList,
            m_pd3dGraphicsRootSignature,
            "Model/SuperCobra.bin", 0.5f
        );

        for(int i=0; i<enemy_count; ++i) {
            XMFLOAT3 position { Random::RandF(-50.0f, 50.0f), Random::RandF(20.0f, 30.0f), Random::RandF(-50.0f, 50.0f) };
            float rotation_speed = Random::RandF(30.0f, 60.0f);

            FlyingObject* enemy = nullptr;

            enemy = new HellicopterObject { };
            enemy->SetChild(helli_model, true);
            enemy->OnInitialize();
            enemy->setBulletMesh(bullet_mesh);

            enemy->SetPosition(position);
            enemy->rotate_local = true;
            enemy->Rotate(0.0f, Random::RandF(0.0f, 360.0f), 0.0f);
            enemy->SetRotationSpeed(rotation_speed / 2.0f);
            enemy->setMovingSpeed(rotation_speed / 6.0f);

            auto oobb = enemy->getBoundingBox();
            oobb.Center.y += 0.5f;
            oobb.Extents.z *= 2.0f;
            enemy->setBoundingBox(oobb.Center, oobb.Extents);

            m_pObjects.push_back(enemy);
            enemies.push_back(enemy);
        }
    }

    CMeshLoadInfo cube_info = CMeshLoadInfo::CubeInfo(0.2f, 0.2f, 0.2f);
    CMesh* cube_mesh = new CMeshIlluminatedFromFile { pd3dDevice, pd3dCommandList, &cube_info };

    CExplosiveObject::PrepareExplosion(pd3dDevice, pd3dCommandList, cube_mesh);

    CreateShaderVariables(pd3dDevice, pd3dCommandList);
}


void GameScene::AnimateObjects(float fTimeElapsed) {
    movePlayer(fTimeElapsed);

    {
        // 플레이어를 지형에 맞게 회전하지 않으므로, 포탑 회전은 더 쉽다.
        auto cannon_direction = m_pPlayer->getCannonLook();
        auto look_direction = m_pPlayer->getLookDirection();

        {
            float look_angle = -m_pPlayer->getLookDirectionPitch();
            float cannon_angle = m_pPlayer->getCannonAngle();
            float angle = look_angle - cannon_angle;
            if(abs(angle) < 1.0f) angle = 0.0f;
            m_pPlayer->rotateCannon(fTimeElapsed, angle);
        }

        {
            auto turret_direction = cannon_direction;
            turret_direction.y = 0.0f;
            turret_direction = Vector3::Normalize(turret_direction);

            look_direction.y = 0.0f;
            look_direction = Vector3::Normalize(look_direction);

            float angle = acosf(Vector3::DotProduct(turret_direction, look_direction));
            angle = XMConvertToDegrees(angle);
            if(angle > 1.0f) {
                if(Vector3::CrossProduct(turret_direction, look_direction).y < 0.0f) {
                    angle = -angle;
                }
                m_pPlayer->rotateTurret(fTimeElapsed, angle);
            }
        }
    }

    Scene::AnimateObjects(fTimeElapsed);

    if(player_searchlight) {
        player_searchlight->m_xmf3Position = m_pPlayer->getCannonPosition();
        player_searchlight->m_xmf3Direction = m_pPlayer->getCannonLook();
    }

    for(int i=0; i<enemy_lights.size(); ++i) {
        XMFLOAT3 to_player = Vector3::Subtract(m_pPlayer->GetPosition(), enemies[i]->GetPosition());
        XMFLOAT3 to_player_horizontal = to_player;
        to_player_horizontal.y = 0.0f;
        float horizontal_distance = Vector3::Length(to_player_horizontal);
        if(horizontal_distance < 50.0f) {
            enemy_lights[i]->m_xmf3Position = enemies[i]->GetPosition();
            enemy_lights[i]->m_xmf3Direction = Vector3::Normalize(to_player);
        }
        else {
            enemy_lights[i]->m_xmf3Position = enemies[i]->GetPosition();
            enemy_lights[i]->m_xmf3Direction = XMFLOAT3 { 0.0f, -1.0f, 0.0f };
        }
    }

    checkEnemyByBulletCollisions();

    updateCamera();
}


bool GameScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
    switch(nMessageID) {
        case WM_LBUTTONDOWN: {
            mouse_pressed = true;
            GetCursorPos(&old_cursor_pos);
            return true;
        }
        case WM_LBUTTONUP:
            mouse_pressed = false;
            break;
        case WM_RBUTTONDOWN:
            camera->switchTo(CCamera::Mode::FirstPerson);
            break;
        case WM_RBUTTONUP:
            camera->switchTo(CCamera::Mode::ThirdPerson);
            break;
        case WM_MOUSEMOVE: if(mouse_pressed) {
            POINT ptCursorPos;
            GetCursorPos(&ptCursorPos);
            float cxMouseDelta = (float)(ptCursorPos.x - old_cursor_pos.x) / 3.0f;
            float cyMouseDelta = (float)(ptCursorPos.y - old_cursor_pos.y) / 3.0f;
            if(cxMouseDelta || cyMouseDelta) {
                m_pPlayer->rotateLookDirection(cyMouseDelta, cxMouseDelta);
            }
            SetCursorPos(old_cursor_pos.x, old_cursor_pos.y);
            break;
        }
        default:
            break;
    }

    return false;
}

bool GameScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
    switch(nMessageID) {
        case WM_KEYUP:
            switch(wParam) {
                case VK_ESCAPE:
                    SendMessage(hWnd, QUIT_GAME_SCENE, 0, 0);
                    return true;
                default:
                    break;
            }
            break;
        case WM_KEYDOWN:
        default:
            break;
    }

    return false;
}


void GameScene::Render(ID3D12GraphicsCommandList* pd3dCommandList) {
    Scene::Render(pd3dCommandList);
}


void GameScene::checkEnemyByBulletCollisions() {
    for(auto& enemy : enemies) {
        if(enemy->is_active) {
            if(m_pPlayer->checkBulletCollision(enemy)) {
                //enemy->is_active = false;
            }
        }
    }
}


void GameScene::movePlayer(float fTimeElapsed) {
    if(m_pPlayer->m_bBlowingUp) return;

    UCHAR pKeyBuffer[256];
    if(GetKeyboardState(pKeyBuffer)) {
        XMFLOAT3 xmf3Shift = { 0.0f, 0.0f, 0.0f };
        if(pKeyBuffer['W'] & 0xF0) xmf3Shift = Vector3::Add(xmf3Shift, m_pPlayer->GetLook());
        if(pKeyBuffer['S'] & 0xF0) xmf3Shift = Vector3::Subtract(xmf3Shift, m_pPlayer->GetLook());

        xmf3Shift = Vector3::ScalarProduct(xmf3Shift, player_speed * fTimeElapsed);

        if(pKeyBuffer['A'] & 0xF0) m_pPlayer->rotateY(-player_rotation_speed * fTimeElapsed);
        if(pKeyBuffer['D'] & 0xF0) m_pPlayer->rotateY(player_rotation_speed * fTimeElapsed);

        if(pKeyBuffer[VK_SPACE] & 0xF0) m_pPlayer->fire();

        //// 언덕이면 속도가 줄어들도록
        //// 이동방향과, 지형의 노멀벡터를 구해서
        //// 45도 이상이면 못올라가도록(오히려 흘러내려오도록)
        //XMFLOAT3 position = m_pPlayer->GetPosition();
        //XMFLOAT3 v = m_pPlayer->GetVelocity();  // 플레이어는 어차피 y방향 속도를 갖지 않도록 할 예정
        //float length = Vector3::Length(v);
        //
        //// 지형의 노멀의 y는 필요 없다.
        //XMFLOAT3 terrain_normal = terrain_height_map->getNormal(m_pPlayer->GetPosition().x, m_pPlayer->GetPosition().z);
        //terrain_normal.y = 0.0f;

        m_pPlayer->Move(xmf3Shift, true);
    }
}


void GameScene::updateCamera() {
    if(m_pPlayer->m_bBlowingUp) return;

    XMFLOAT3 up { 0.0f, 1.0f, 0.0f };
    XMFLOAT3 look = m_pPlayer->getLookDirection();
    XMFLOAT3 right = Vector3::CrossProduct(up, look, true);
    XMFLOAT3 position = m_pPlayer->getTurretPosition();
    XMFLOAT3 offset = camera->getOffset();
    XMFLOAT3 look_at = Vector3::Add(position, Vector3::ScalarProduct(look, offset.z+1.0f, false));
    
    position = Vector3::Add(position, Vector3::ScalarProduct(look, offset.z, false));
    position = Vector3::Add(position, Vector3::ScalarProduct(up, offset.y, false));
    position = Vector3::Add(position, Vector3::ScalarProduct(right, offset.x, false));
    
    look_at = Vector3::Add(look_at, Vector3::ScalarProduct(up, offset.y, false));
    look_at = Vector3::Add(look_at, Vector3::ScalarProduct(right, offset.x, false));

    camera->GenerateViewMatrix(position, look_at, up);
}
