<br>

[간단한 영상](https://youtu.be/wvUPG6sIBE4)

<br>

<br>

**DirectX11 + IOCP 연동 습작 (제작중)**

IOCP서버 코드와 코드에 대한 대략적인 설명은<br>
(https://github.com/homodeus-ms/IOCP-portfolio)<br>
이쪽을 봐주시기 바랍니다.<br>

현재까지 이동 동기화, 충돌 처리, Picking, 타겟팅, 공격등이 구현 되었습니다.<br>

서버 기술로는 IOCP, Job방식의 비동기처리, 메모리 풀링등이 사용 되었습니다.<br>
메인스레드가 게임 루프를 돌고 워커스레드들이 JobQueue의 일감들을 꺼내어 처리하는 방식입니다.<br> 

Rendering과 충돌 처리를 원할히 위해서 Octree와 Frustum Culling, Instancing Render, Billboard 등이 사용되었습니다.<br>

<br>

<br>

---

<br>

**이동동기화**

WASD로 클라이언트에서 먼저 움직이고 서버에서 Valid 움직임 체크 후 클라이언트에게 다시 응답하는 방식입니다.

<br>

![Move 방식](ReadMeImages/move.jpg)

<br>

<br>

**<관련 클래스>**

**엔진**

Octree, BoundingCube<br>
[Octree folder](Engine/Octree/)<br>

**클라이언트**

Player, MyPlayer &nbsp; [ClientObject folder](Client/ClientObject/)<br>
DevApp1 &nbsp;[DevApp1 folder](Client/Main/)<br>

**서버**

GameRoom, MovePacketHandler, RttRecorder<br>
[Room folder](Server/Room)<br>

<br>

---

<br>

**<충돌 처리>**

<br>

1. 옥트리 서치(최소 큐브 크기, 혹은 재귀 Depth로 깊이 제한)<br>
2. 옥트리로 구현 했으나, 높이(y축)를 거의 사용하지 않는 다면 쿼드트리로 충분할 것 같습니다.<br>
3. 클라쪽과 서버 양쪽에서 모두 충돌 검사를 하고 있습니다.
   - 클라쪽에서 움직이려고 할 때 충돌 검사를 하고 충돌이 발생하면 아예 패킷을 보내지 않습니다.
   - 그렇지만 최종 검증은 서버에서 해야 된다고 생각했습니다.<br>
4. 충돌 검사와 관련된 함수들은 Directx의 라이브러리를 사용했습니다.<br>

**<관련 클래스>**

**엔진**

Octree, BoundingCube<br>
[Octree folder](Engine/Octree/)<br>

Scene.cpp의 Frustum culling, Render 부분<br>
[Scene.cpp, Update()](Engine/Scene/Scene.cpp#L37)<br>

**서버**















