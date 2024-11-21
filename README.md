<br>

[Watch this video](https://youtu.be/wvUPG6sIBE4)

<br>

**이하, 포트폴리오에 대한 설명입니다.**

<br>

**DirectX11 + IOCP 연동 습작 (제작중)**

IOCP서버 코드와 코드에 대한 대략적인 설명은
[링크] (https://github.com/homodeus-ms/IOCP-portfolio)
이쪽을 봐주시기 바랍니다.

현재까지 이동 동기화, 충돌 처리, 타겟팅, 공격등이 구현 되었습니다.

서버 기술로는 IOCP, Job방식의 비동기처리, 메모리 풀링등이 사용 되었습니다.

원할한 처리를 위해서 Octree와 Frustum Culling, Instancing Render, Billboard 등이 사용되었습니다.

<br>

<br>

**이동동기화**

WASD로 클라이언트에서 먼저 움직이고 서버에서 Valid 움직임 체크 후 클라이언트에게 다시 응답하는 방식입니다.

<br>

![Move 방식](ReadMeImages/move.jpg)

<br>

<br>

**관련 클래스**

**엔진**

[Octree.h](Engine/Octree/Octree.h) &nbsp;&nbsp; [Octree.cpp](Engine/Octree/Octree.cpp)
<br>
[BoundingCube.h](Engine/Octree/BoundingCube.h) &nbsp;&nbsp; [BoundingCube.cpp](Engine/Octree/BoundingCube.cpp)
<br>

**클라이언트**

[Player.h](Client/ClientObject/Player.h) &nbsp;&nbsp; [Player.cpp](Client/ClientObject/Player.cpp) &nbsp;&nbsp; 클라이언트, Player
<br>
[MyPlayer.h](Client/ClientObject/MyPlayer.h) &nbsp;&nbsp; [MyPlayer.cpp](Client/ClientObject/MyPlayer.cpp) &nbsp;&nbsp; 클라이언트, MyPlayer
<br>
[DevApp1.h](Client/Main/DevApp1.h) &nbsp;&nbsp; [DevApp1.cpp](Client/Main/DevApp1.cpp) &nbsp;&nbsp; 클라이언트, DevApp1(총괄 클래스, DevScene같은)
<br>

**서버**

[GameRoom.h](Server/Room/)















