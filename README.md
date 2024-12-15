# 📷 CCTV를 이용한 하이패스 기능 구현

## 📖 소개
하이패스를 이용할 경우 발생할 수 있는 사고 위험을 방지하고, CCTV를 활용하여 하이패스 기능을 대체함으로써 인건비 절감, 사고 예방, 교통 체증 감소를 목표로 하는 프로젝트입니다.

## 🚀 주요 기능

### 클라이언트
- 실시간 스트리밍 화면 출력
- 요금 계산 및 데이터 조회 기능 제공
- 번호판에 따른 사용자 이메일 등록/조회 후 청구서 발송

## 🛠️ 설치 및 실행
### 요구 사항
- Qt 4 이상
- Windows 또는 Ubuntu
- Ubuntu 사용 시 FFmpeg 설치 필요:
```bash
sudo apt install ffmpeg
설치 및 실행
프로젝트 클론
git clone https://github.com/VedaHighPass/veda-qt-highpass-client.git
cd veda-qt-highpass-client/qtclient
demo_vars.h 파일 작성
아래 예시를 참고하여 demo_vars.h 파일 생성:

#ifndef DEMO_VARS_H
#define DEMO_VARS_H

#define SMTP_SERVER "smtp.gmail.com"

#define SENDER_NAME "발신자 이름"
#define SENDER_EMAIL "sender@gmail.com"
#define SENDER_PASSWORD "구글 앱 비밀번호"

#define RECIPIENT_NAME "수신자 이름"
#define RECIPIENT_EMAIL "수신자 이메일"

#endif

httpclient.cpp 수정
HttpClient::loadCameras() 함수에서 서버 URL을 데이터베이스 서버 주소로 수정해야 합니다.
예를 들어:
QUrl url("https://192.168.0.26:8080/cameras");

위 URL을 현재 사용하는 데이터베이스 서버 주소로 변경하십시오.

빌드 및 실행

# 서버 실행 후 클라이언트 실행
cd veda-qt-highpass-client/qtclient
qmake
make
./qtclient

📜 라이선스
이 프로젝트는 GPL (GNU General Public License) 2.0 이상 버전의 라이선스를 따릅니다. 자세한 내용은 LICENCE 파일을 참조하세요.
