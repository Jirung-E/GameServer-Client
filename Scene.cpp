#include "Scene.h"

#include "Shader.h"
#include "../GameServer-Server/game_header.h"

#include <iostream>


Scene::Scene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList):
    scene_width { FRAME_BUFFER_WIDTH },
    scene_height { FRAME_BUFFER_HEIGHT },
    camera { nullptr },
    m_pd3dDevice { pd3dDevice },
    m_pd3dCommandList { pd3dCommandList },
    m_xmf4GlobalAmbient { 0.0f, 0.0f, 0.0f, 1.0f }
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


ID3D12RootSignature* Scene::CreateGraphicsRootSignature() {
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

    m_pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(),
        pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), 
        (void**)&pd3dGraphicsRootSignature);

    if(pd3dSignatureBlob) pd3dSignatureBlob->Release();
    if(pd3dErrorBlob) pd3dErrorBlob->Release();

    return pd3dGraphicsRootSignature;
}

ID3D12RootSignature* Scene::GetGraphicsRootSignature() {
    return m_pd3dGraphicsRootSignature;
}


void Scene::CreateShaderVariables() {
    UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256�� ���
    m_pd3dcbLights = ::CreateBufferResource(
        m_pd3dDevice, m_pd3dCommandList, 
        nullptr, ncbElementBytes, 
        D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, 
        nullptr
    );

    m_pd3dcbLights->Map(0, nullptr, (void**)&m_pcbMappedLights);
}

void Scene::UpdateShaderVariables() {
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

void Scene::BuildObjects() {
    m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature();
}

void Scene::ReleaseObjects() {
    for(CGameObject* obj : m_pObjects) {
        obj->Release();
    }

    if(m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
    ReleaseShaderVariables();

    if(m_pLights) delete[] m_pLights;
}


void Scene::AnimateObjects(float fTimeElapsed) {
    m_fElapsedTime = fTimeElapsed;

    for(CGameObject* obj : m_pObjects) {
        obj->Animate(fTimeElapsed, NULL);
    }
}



void Scene::Render() {
    m_pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

    camera->SetViewportsAndScissorRects(m_pd3dCommandList);
    camera->UpdateShaderVariables(m_pd3dCommandList);

    UpdateShaderVariables();

    D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
    m_pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights

    //���� �������ϴ� ���� ���� �����ϴ� ���� ��ü(���̴��� �����ϴ� ��ü)���� �������ϴ� ���̴�.
    for(CGameObject* obj : m_pObjects) {
        obj->Animate(m_fElapsedTime, NULL);
        obj->UpdateTransform(NULL);
        obj->Render(m_pd3dCommandList, camera);
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





















GameScene::GameScene(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList): 
    Scene { pd3dDevice, pd3dCommandList },
    console { },
    wsa_guard { },
    tcp_connection { },
    client_id { },
    name { },
    packet_parser { }
{
    bg_color = { 0.0f, 0.125f, 0.3f, 1.0f };
    near_plane_distance = 0.1f;
    far_plane_distance = 500.0f;

    connectToServer();
    login();

    console.close();
}

GameScene::~GameScene() {
    black_material->Release();
    white_material->Release();
    gray_material->Release();
}


void GameScene::CreateShaderVariables() {
    Scene::CreateShaderVariables();

    camera->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
}

void GameScene::ReleaseUploadBuffers() {
    Scene::ReleaseUploadBuffers();

    for(auto& player : players) {
        player.second->ReleaseUploadBuffers();
    }
}


void GameScene::BuildDefaultLightsAndMaterials() {
    m_nLights = 1;        // �¾�
    m_pLights = new LIGHT[m_nLights];
    ::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

    m_xmf4GlobalAmbient = XMFLOAT4 { 0.1f, 0.1f, 0.1f, 1.0f };

    m_pLights[0].m_bEnable = true;
    m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
    m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_pLights[0].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_pLights[0].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_pLights[0].m_xmf3Direction = Vector3::Normalize(XMFLOAT3(1.0f, -2.4f, -1.0f));

    CMaterialColors* black_metal = new CMaterialColors { };
    black_metal->m_xmf4Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    black_metal->m_xmf4Diffuse = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
    black_metal->m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.95f);
    black_metal->m_xmf4Emissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    black_material = new CMaterial { };
    black_material->SetMaterialColors(black_metal);
    black_material->SetIlluminatedShader();
    black_material->AddRef();

    CMaterialColors* white_metal = new CMaterialColors { };
    white_metal->m_xmf4Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    white_metal->m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    white_metal->m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.7f);
    white_metal->m_xmf4Emissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    white_material = new CMaterial { };
    white_material->SetMaterialColors(white_metal);
    white_material->SetIlluminatedShader();
    white_material->AddRef();

    CMaterialColors* gray_metal = new CMaterialColors { };
    gray_metal->m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    gray_metal->m_xmf4Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    gray_metal->m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.7f);
    gray_metal->m_xmf4Emissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    gray_material = new CMaterial { };
    gray_material->SetMaterialColors(gray_metal);
    gray_material->SetIlluminatedShader();
    gray_material->AddRef();
}

void GameScene::BuildObjects() {
    Scene::BuildObjects();

    CMaterial::PrepareShaders(m_pd3dDevice, m_pd3dCommandList, m_pd3dGraphicsRootSignature);

    BuildDefaultLightsAndMaterials();

    {
        camera = new CCamera { };
        adaptCamera();
    }

    {
        player_mesh = CPlayer::LoadMeshFromFile(m_pd3dDevice, m_pd3dCommandList, "Model/king.obj", 0.5f);
    }

    {
        CMeshLoadInfo cube_info = CMeshLoadInfo::CubeInfo(1.0f, 1.0f, 1.0f);
        CMesh* cube_mesh = new CMeshIlluminatedFromFile { m_pd3dDevice, m_pd3dCommandList, &cube_info };

        for(int i=0; i<8; ++i) {
            for(int k=0; k<8; ++k) {
                CubeObject* cube = new CubeObject { };
                cube->SetMesh(cube_mesh);
                cube->SetPosition(static_cast<float>(k), -0.5f, static_cast<float>(i));
                if((i + k) % 2 == 0) {
                    cube->SetMaterial(0, black_material);
                }
                else {
                    cube->SetMaterial(0, white_material);
                }

                m_pObjects.push_back(cube);
            }
        }
    }

    CreateShaderVariables();
}

void GameScene::ReleaseObjects() {
    for(auto& player : players) {
        player.second->Release();
    }

    if(camera) delete camera;

    Scene::ReleaseObjects();
}


void GameScene::AnimateObjects(float fTimeElapsed) {
    recvFromServer();

    movePlayer(fTimeElapsed);

    Scene::AnimateObjects(fTimeElapsed);
    for(auto& player : players) {
        player.second->Animate(fTimeElapsed, nullptr);
    }

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
            break;
        case WM_RBUTTONUP:
            break;
        case WM_MOUSEMOVE: if(mouse_pressed) {
            POINT ptCursorPos;
            GetCursorPos(&ptCursorPos);
            float cxMouseDelta = (float)(ptCursorPos.x - old_cursor_pos.x) / 3.0f;
            float cyMouseDelta = (float)(ptCursorPos.y - old_cursor_pos.y) / 3.0f;
            if(cxMouseDelta || cyMouseDelta) {
                
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
        case WM_KEYDOWN: {
            switch(wParam) {
                case VK_RIGHT: {
                    move(MOVE_RIGHT);
                    return true;
                }
                case VK_LEFT: {
                    move(MOVE_LEFT);
                    return true;
                }
                case VK_UP: {
                    move(MOVE_UP);
                    return true;
                }
                case VK_DOWN: {
                    move(MOVE_DOWN);
                    return true;
                }
            }
        }
        default:
            break;
    }

    return false;
}


void GameScene::Render() {
    Scene::Render();

    for(auto& player : players) {
        player.second->Animate(m_fElapsedTime, NULL);
        player.second->UpdateTransform(NULL);
        player.second->Render(m_pd3dCommandList, camera);
    }
}


CPlayer* GameScene::addPlayer(long long id, const float x, const float z) {
    CPlayer* player = new CPlayer { };
    player->SetMesh(player_mesh);
    player->SetPosition({ x, 0.0f, z });
    players.insert({ id, player });
    return player;
}

void GameScene::movePlayer(float fTimeElapsed) {
    UCHAR pKeyBuffer[256];
    if(GetKeyboardState(pKeyBuffer)) {
        XMFLOAT3 xmf3Shift = { 0.0f, 0.0f, 0.0f };
        if(pKeyBuffer['W'] & 0xF0) xmf3Shift = Vector3::Add(xmf3Shift, m_pPlayer->GetLook());
        if(pKeyBuffer['S'] & 0xF0) xmf3Shift = Vector3::Subtract(xmf3Shift, m_pPlayer->GetLook());

        xmf3Shift = Vector3::ScalarProduct(xmf3Shift, player_speed * fTimeElapsed);

        //if(pKeyBuffer['A'] & 0xF0) m_pPlayer->rotateY(-player_rotation_speed * fTimeElapsed);
        //if(pKeyBuffer['D'] & 0xF0) m_pPlayer->rotateY(player_rotation_speed * fTimeElapsed);

        //m_pPlayer->Move(xmf3Shift, true);
    }
}


void GameScene::updateCamera() {
    XMFLOAT3 up { 0.0f, 1.0f, 0.0f };
    XMFLOAT3 position { 3.5f, 5.0f, 3.4f };
    XMFLOAT3 look_at { 3.5f, 0.0f, 3.5f };

    camera->GenerateViewMatrix(position, look_at, up);
}


void GameScene::connectToServer() {
    //std::cout << "Enter Address(ip): ";
    //std::string addr;
    //std::cin >> addr;
    std::string addr = "127.0.0.1";

    std::cout << "connecting to " << addr << ":" << 3000 << std::endl;

    tcp_connection.connect(addr);
    tcp_connection.setNoBlock(true);
}

void GameScene::login() {
    //std::cout << "Enter your name(max " << static_cast<int>(MAX_ID_LENGTH) << " character): ";
    //std::string name;
    //std::cin >> name;
    std::string name = "player0";

    cs_packet_login lp { name };
    tcp_connection.send(reinterpret_cast<Packet*>(&lp));
}

void GameScene::move(char direction) {
    cs_packet_move mp { direction };
    tcp_connection.send(reinterpret_cast<Packet*>(&mp));
}

void GameScene::recvFromServer() {
    char buf[1024] = { 0 };
    DWORD size_recv = 0;
    auto res = tcp_connection.receive(buf, sizeof(buf), &size_recv);
    if(res == SOCKET_ERROR) {
        auto err_no = WSAGetLastError();
        if(err_no != WSAEWOULDBLOCK) {
            TcpConnection::printErrorMessage(err_no);
            return;
        }
    }

    packet_parser.push(buf, size_recv);

    while(packet_parser.canPop()) {
        Packet packet = packet_parser.pop();
        processPacket(packet);
    }
}

void GameScene::processPacket(Packet& packet) {
    switch(packet.type) {
        case S2C_P_AVATAR_INFO: {
            sc_packet_avatar_info* info_packet = reinterpret_cast<sc_packet_avatar_info*>(&packet);
            client_id = info_packet->id;
            float x = static_cast<float>(info_packet->x);
            float z = static_cast<float>(info_packet->y);
            m_pPlayer = addPlayer(client_id, x, z);
            m_pPlayer->SetMaterial(0, white_material);
            break;
        }
        case S2C_P_ENTER: {
            sc_packet_enter* enter_packet = reinterpret_cast<sc_packet_enter*>(&packet);
            int client_id = static_cast<int>(enter_packet->id);
            if(players.find(client_id) == players.end()) {
                float x = static_cast<float>(enter_packet->x);
                float z = static_cast<float>(enter_packet->y);
                auto player = addPlayer(client_id, x, z);
                player->SetMaterial(0, gray_material);
            }
            break;
        }
        case S2C_P_MOVE: {
            sc_packet_move* move_packet = reinterpret_cast<sc_packet_move*>(&packet);
            int client_id = static_cast<int>(move_packet->id);
            if(players.find(client_id) != players.end()) {
                float x = static_cast<float>(move_packet->x);
                float z = static_cast<float>(move_packet->y);
                auto player = players[client_id];
                player->SetPosition({ x, 0.0f, z });
            }
            break;
        }
        case S2C_P_LEAVE: {
            sc_packet_leave* leave_packet = reinterpret_cast<sc_packet_leave*>(&packet);
            int client_id = static_cast<int>(leave_packet->id);
            auto it = players.find(client_id);
            if(it != players.end()) {
                CPlayer* player = it->second;
                players.erase(it);
                player->Release();
            }
            break;
        }
        default:
            std::cout << "unknown packet type: " << packet.type << std::endl;
            break;
    }
}
