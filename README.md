[Watch this video](https://www.youtube.com/watch?v=FNH4_RnqMBU)

<br>


**DirectX11 + IOCP 연동 제작중 데모입니다(한참 미완성)**

IOCP서버 코드는
[링크] (https://github.com/homodeus-ms/IOCP-portfolio)
이쪽에 있습니다.

DirectX11 엔진코드는 인프런의 Rookiss님 강의를 참고로 작성했기에 첨부하진 않았습니다.

DirectX를 학습 중이었는데 취업 지원을 위해서 급하게 포폴을 제작해 보느라 아직 미흡한 점이 상당히 많습니다.

계속해서 조금씩 업데이트 해 나갈 예정입니다. 취업이 그리 쉽지는 않을테니까요.

현재는 3D애니메이션 캐릭터와 모델들을 맵에 추가하고 방향키에 따라 이동, 회전, 점프를 구현되었고

많은 3D 물체를 그리고 충돌처리를 하기 위해서 Instancing과 Octree를 구현했습니다.

<br>

충돌을 서버에서 판단해서 클라로 보내주는 데 해당 코드들은

Server폴더의

- Octree - Octree.cpp
- Room - GameRoom.cpp 의 TryMove 함수쪽에 있습니다.

Instancing부분은 강의를 참고한 부분이지만 

- Client - Engine 부분에 있습니다.


<br>

<br>

아직 다른 클라이언트가 접속하면 화면에 등장하고 움직임이 연동이 되지만 불완전합니다.

제작하면서 계속 Loop Back Address 로 테스트 해서 잘 되고 있다고 착각 했었습니다.

막상 다른 컴퓨터와 연결을 해보니 움직임이 엄청 끊깁니다.

패킷전송 속도를 고려를 못했습니다.

부드러운 움직임을 만들기 위해서 현재위치와 전송받은 위치를 고려해서 보간작업을 해야한다고 알고 있습니다.

읽어주셔서 감사합니다.


