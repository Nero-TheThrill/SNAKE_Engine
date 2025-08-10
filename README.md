[![Latest release](https://img.shields.io/github/v/release/Nero-TheThrill/SNAKE_ENGINE)](https://github.com/Nero-TheThrill/SNAKE_ENGINE/releases)
[![Changelog](https://img.shields.io/badge/Changelog-keep%20a%20changelog-blue)](./CHANGELOG.md)

> Full changes: [CHANGELOG.md](./CHANGELOG.md)

# SNAKE\_ENGINE

A lightweight C++ game engine (sample framework) that bundles 2D rendering, input, sound, text, collision, and state management in one place. Built on OpenGL 4.6, it supports batched rendering and instancing, and handles text via runtime glyph baking (FreeType). It provides a dependency/tag–based resource system so you can pull in only the modules your project needs.

---

## Key Features

* **State transition framework:** Separates the lifecycle into Load/Init/Update/Draw/Free/Unload per `GameState`.
* **Camera & frustum culling:** `Camera2D` coordinate system + automatic exclusion of off–screen objects.
* **Batching & instancing:** Groups identical Mesh/Material pairs; instancing based on the `i_Model` attribute.
* **Runtime text engine:** Bakes glyphs on demand via FreeType; supports alignment and multi-line text.
* **Input utilities:** Compare current/previous key/mouse states, convert between screen/world coordinates, read scroll delta.
* **Collision & broad phase:** AABB/Circle colliders + spatial hash grid to minimize pair comparisons.
* **Sound:** FMOD-based play/pause/stop, instance ID management, tag-level controls.
* **Debug draw:** Line-drawing API (with camera/projection) to visualize colliders, etc.
* **Resource registry:** Tag-based register/lookup/release for Shader/Texture/Mesh/Material/Font/SpriteSheet.

---

## Engine Architecture Overview

```
SNAKE_Engine
 ├─ WindowManager : GLFW/GLAD init, event callbacks, swap/clear
 ├─ InputManager  : Key/mouse state, scroll, world-coordinate conversion
 ├─ SoundManager  : FMOD system, sound loading/playback/control
 ├─ RenderManager : Resource registry, batching/instancing, debug draw
 │   ├─ RenderLayerManager : Manages 0–16 layer tags
 │   └─ FrustumCuller      : Camera-based visibility filtering
 ├─ StateManager  : Switch/update/draw for GameState
 └─ (GameState)   : Owns ObjectManager + CameraManager
      └─ ObjectManager : Object creation/lifetime/collision/draw submit
          ├─ Object    : Transform2D, Mesh/Material/Animator/Collider
          ├─ TextObject: Font + text mesh generation
          └─ Collider  : Circle/AABB + SpatialHashGrid
```

---

## Requirements

* C++17 or later, CMake (recommended)
* OpenGL 4.6 driver
* Platform libraries: **GLFW**, **GLAD**, **GLM**, **stb\_image**, **FreeType**, **FMOD Core API**

> FMOD has separate licensing requirements for commercial use. Please check the license of each library.

---

## Usage by System

### 1) Rendering / Resources

* **Tag register/lookup:** Use `Register*` / `Get*ByTag`. Re-registering the same tag warns and is ignored.

* **Material uniforms/textures:**

  ```cpp
  Material* m = engineContext.renderManager->GetMaterialByTag("mat.brick");
  m->SetUniform("u_Color", glm::vec4(1,1,1,1));
  m->SetTexture("u_Texture", engineContext.renderManager->GetTextureByTag("brick"));
  ```

* **Instancing:** Your shader must expose the `i_Model` attribute. After enabling, same Mesh/Material pairs are drawn in one call.

  ```cpp
  Material* m = engineContext.renderManager->GetMaterialByTag("mat.instanced");
  Mesh* mesh = engineContext.renderManager->GetMeshByTag("[EngineMesh]default");
  m->EnableInstancing(true, mesh);
  ```

### 2) Text / Fonts

```cpp
engineContext.renderManager->RegisterFont("NotoSans16", "fonts/NotoSans-Regular.ttf", 16);
Font* font = engineContext.renderManager->GetFontByTag("NotoSans16");
auto text = std::make_unique<TextObject>(font, "Hello, World!", TextAlignH::Center, TextAlignV::Middle);
text->SetColor({0.4,0.7,1.0,0.7});
text->SetIgnoreCamera(true, engineContext.stateManager->GetCurrentState()->GetActiveCamera());
objects.AddObject(std::move(text), "title");
```

* Text bakes only the needed glyphs into the atlas at runtime, and supports alignment/multi-line.

### 3) Animation (Sprite Sheets)

```cpp
engineContext.renderManager->RegisterTexture("hero", "Textures/hero.jpg");
engineContext.renderManager->RegisterSpriteSheet("heroSpriteSheet", "hero", 32, 32);
SpriteSheet* sheet = engineContext.renderManager->GetSpriteSheetByTag("heroSpriteSheet");
sheet->AddClip("sidewalk",  { 0,1,2,3,4,5,6,7,8 }, 0.08f, true);
sheet->AddClip("frontwalk", { 86,87,88,89,90,91 }, 0.08f, true);
sheet->AddClip("idle",      { 9 },                 0.08f, false);
AttachAnimator(sheet, 0.08f);
GetSpriteAnimator()->PlayClip("idle");
```

### 4) Input

```cpp
if (engineContext.inputManager->IsKeyPressed(GLFW_KEY_SPACE)) { /* ... */ }
```

### 5) Camera / Layers

```cpp
engineContext.renderManager->RegisterRenderLayer("UI", 2);

Camera2D* cam = cameraManager.GetCamera("main"); // default camera
cam->SetZoom(1.25f);
cam->SetPosition({100,50});

obj->SetRenderLayer("UI");
```

### 6) Collision

```cpp
obj->SetCollider(std::make_unique<AABBCollider>(obj, glm::vec2(1.f, 1.f)));
obj->GetCollider()->SetUseTransformScale(true);
obj->SetCollision(engineContext.stateManager->GetCurrentState()->GetObjectManager(),
                  "button", { "player" });
```

* Supports circle/box colliders, point tests, and debug rendering.

### 7) Sound

```cpp
engineContext.soundManager->LoadSound("bgm", "audio/bgm.ogg", /*loop=*/true);
auto id = engineContext.soundManager->Play("bgm", 0.7f);
engineContext.soundManager->ControlByID(SoundManager::SoundControlType::Pause, id);
engineContext.soundManager->ControlByTag(SoundManager::SoundControlType::Resume, "bgm");
```

### 8) Debug Draw

```cpp
engineContext.renderManager->DrawDebugLine({-50,-50}, {50,50}, GetActiveCamera(), {1,0,0,1}, 2.0f);
// StateManager flushes every frame; shown when the engine allows debug draws
```

---

## Performance Tips

* Maximize identical **Mesh/Material** combinations to reduce draw calls.
* Use **instancing** whenever possible.
* For large maps, **spatial hashing** and **frustum culling** provide significant gains in collision/visibility.

---

## Troubleshooting

* Seeing a black/yellow checker texture? Your material doesn’t have a texture bound.
* Garbled fonts? Check the TTF path/pixel size range and logs to ensure the atlas expands dynamically.

---

## Third-Party

* **GLFW** — window/input
* **GLAD** — OpenGL loader
* **OpenGL 4.6** — rendering
* **GLM** — math
* **stb\_image** — image loader
* **FreeType** — fonts/glyphs
* **FMOD Core API** — audio

Please comply with each library’s license.


---
---
---


# SNAKE\_ENGINE

가벼운 2D 렌더링·입력·사운드·텍스트·충돌·상태 관리를 한 곳에 묶은 C++ 게임 엔진(샘플 프레임워크)입니다. OpenGL 4.6 기반으로 배치 렌더링과 인스턴싱을 지원하며, 텍스트는 런타임 글리프 베이킹(FreeType)으로 처리합니다. 프로젝트 성격에 맞게 필요한 모듈만 가져다 쓰도록 의존성/태그 기반 리소스 시스템을 제공합니다.

---

## 핵심 특징

* **상태(State) 전환 프레임워크**: `GameState` 단위로 Load/Init/Update/Draw/Free/Unload 라이프사이클을 분리.
* **카메라/프러스텀 컬링**: `Camera2D` 좌표계 + 화면 밖 객체 자동 배제.
* **배치·인스턴싱 렌더링**: 동일 메쉬/머티리얼 묶음 처리, `i_Model` 속성 기반 인스턴싱.
* **런타임 텍스트 엔진**: FreeType로 글리프를 필요 시 베이킹, 정렬/멀티라인 지원.
* **입력 유틸리티**: 키/마우스 현재/직전 상태 비교, 화면/월드 좌표 변환, 스크롤 델타.
* **충돌/브로드페이즈**: AABB/원 콜라이더 + 공간 해시 그리드로 쌍 비교 최소화.
* **사운드**: FMOD 기반 재생/일시정지/정지, 인스턴스 ID 관리, 태그별 제어.
* **디버그 드로우**: 선 그리기 API(카메라/투영 적용)로 콜라이더 등 가시화.
* **리소스 레지스트리**: 태그 기반 Shader/Texture/Mesh/Material/Font/SpriteSheet 등록/조회/해제.

---

## 엔진 구조 개요

```
SNAKE_Engine
 ├─ WindowManager : GLFW/GLAD 초기화, 이벤트 콜백, 스왑/클리어
 ├─ InputManager  : 키/마우스 상태 관리, 스크롤, 월드 좌표 변환
 ├─ SoundManager  : FMOD 시스템, 사운드 로딩/재생/제어
 ├─ RenderManager : 리소스 레지스트리, 배치/인스턴싱, 디버그 드로우
 │   ├─ RenderLayerManager : 0~16 레이어 태그 관리
 │   └─ FrustumCuller      : 카메라 기반 시가시성 필터링
 ├─ StateManager  : GameState 교체/업데이트/그리기 호출
 └─ (GameState)   : ObjectManager + CameraManager 소유
      └─ ObjectManager : Object 생성/수명/충돌/드로우 제출
          ├─ Object    : Transform2D, Mesh/Material/Animator/Collider
          ├─ TextObject: Font + 텍스트 메시 생성
          └─ Collider  : Circle/AABB + SpatialHashGrid
```

---

## 요구 사항

* C++17 이상, CMake(권장)
* OpenGL 4.6 드라이버
* 플랫폼 라이브러리: **GLFW**, **GLAD**, **GLM**, **stb\_image**, **FreeType**, **FMOD Core API**

> FMOD는 상업적 사용 시 별도 라이선스 요건이 있습니다. 각 라이브러리의 라이선스를 반드시 확인하세요.


---

## 주요 시스템 사용법

### 1) 렌더링/리소스

* **태그 등록/조회**: `Register*`/`Get*ByTag` 사용. 같은 태그 재등록 시 경고 후 무시.
* **머티리얼 유니폼/텍스처**:

  ```cpp
  Material* m = engineContext.renderManager->GetMaterialByTag("mat.brick");
  m->SetUniform("u_Color", glm::vec4(1,1,1,1));
  m->SetTexture("u_Texture", engineContext.renderManager->GetTextureByTag("brick"));
  ```
* **인스턴싱**: 셰이더가 `i_Model` 어트리뷰트를 노출해야 함. 활성화 후 동일 메쉬/머티리얼 묶음을 한 번에 드로우.

  ```cpp
  Material* m = engineContext.renderManager->GetMaterialByTag("mat.instanced");
  Mesh* mesh = engineContext.renderManager->GetMeshByTag("[EngineMesh]default");
  m->EnableInstancing(true, mesh);
  ```

### 2) 텍스트/폰트

```cpp
engineContext.renderManager->RegisterFont("NotoSans16", "fonts/NotoSans-Regular.ttf", 16);
Font* font = engineContext.renderManager->GetFontByTag("NotoSans16");
auto text = std::make_unique<TextObject>(font, "Hello, World!", TextAlignH::Center, TextAlignV::Middle);
text->SetColor({0.4,0.7,1.0,0.7});
text->SetIgnoreCamera(true, engineContext.stateManager->GetCurrentState()->GetActiveCamera());
objects.AddObject(std::move(text), "title");
```

* 텍스트는 런타임에 필요한 글리프만 아틀라스에 베이킹하며, 정렬/멀티라인을 지원합니다.

### 3) 애니메이션(스프라이트 시트)

```cpp
engineContext.renderManager->RegisterTexture("hero", "Textures/hero.jpg");
engineContext.renderManager->RegisterSpriteSheet("heroSpriteSheet", "hero", 32, 32);
SpriteSheet* sheet = engineContext.renderManager->GetSpriteSheetByTag("heroSpriteSheet");
sheet->AddClip("sidewalk", { 0,1,2,3,4,5,6,7,8 }, 0.08f, true);
sheet->AddClip("frontwalk", { 86,87,88,89,90,91 }, 0.08f, true);
sheet->AddClip("idle", { 9 }, 0.08f, false);
AttachAnimator(sheet, 0.08f);
GetSpriteAnimator()->PlayClip("idle");

```

### 4) 입력

```cpp
if(engineContext.inputManager->IsKeyPressed(GLFW_KEY_SPACE)) {/*...*/}
```

### 5) 카메라/레이어

```cpp
engineContext.renderManager->RegisterRenderLayer("UI",2);

Camera2D* cam = cameraManager.GetCamera("main"); // 기본 카메라
cam->SetZoom(1.25f);
cam->SetPosition({100,50});

obj->SetRenderLayer("UI");
```

### 6) 충돌

```cpp
obj->SetCollider(std::make_unique<AABBCollider>(obj, glm::vec2(1.f, 1.f)));
obj->GetCollider()->SetUseTransformScale(true);
obj->SetCollision(engineContext.stateManager->GetCurrentState()->GetObjectManager(), "button", { "player" });
```

* 원/사각 콜라이더, 포인트 테스트, 디버그 렌더 지원.

### 7) 사운드

```cpp
engineContext.soundManager->LoadSound("bgm", "audio/bgm.ogg", /*loop=*/true);
auto id = engineContext.soundManager->Play("bgm", 0.7f);
engineContext.soundManager->ControlByID(SoundManager::SoundControlType::Pause, id);
engineContext.soundManager->ControlByTag(SoundManager::SoundControlType::Resume, "bgm");
```

### 8) 디버그 드로우

```cpp
engineContext.renderManager->DrawDebugLine({-50,-50}, {50,50}, GetActiveCamera(), {1,0,0,1}, 2.0f);
// StateManager가 매 프레임 Flush, 엔진이 디버그 드로우 허용 시 화면에 표시됨
```

---

## 성능 팁

* 같은 **Mesh/Material** 조합을 최대화하면 드로우 콜이 줄어듭니다.
* 가능한 경우 **인스턴싱** 사용.
* 큰 맵은 충돌/가시성 측면에서 **스페이셜 해시**와 **프러스텀 컬링**의 효과를 크게 받습니다.

---

## 문제 해결

* 검정/노란 체크 텍스처가 보이면 머티리얼에 텍스처가 바인딩되지 않은 것입니다.
* 폰트가 깨지면 TTF 경로/픽셀 크기 범위를 확인하고, 아틀라스가 동적으로 확장되는지 로그를 보세요.

---

## 서드파티

* **GLFW** — 창/입력
* **GLAD** — OpenGL 로딩
* **OpenGL 4.6** — 렌더링
* **GLM** — 수학
* **stb\_image** — 이미지 로더
* **FreeType** — 폰트/글리프
* **FMOD Core API** — 오디오

각 라이브러리의 라이선스를 준수해 주세요.

---
