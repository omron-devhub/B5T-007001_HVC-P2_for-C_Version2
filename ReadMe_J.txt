----------------------------------------------------
 B5T-007001 �T���v���R�[�h
----------------------------------------------------
(1) �{�񋟕��ɂ���
  B5T-007001(HVC-P2)�̃T���v���R�[�h��񋟂������܂��B
    1-1) B5T-007001�ƃR�}���h����M(�@�\���s)���s���T���v���R�[�h
    1-2) HVC�̃Z���V���O���ʂ����艻���錋�ʈ��艻���C�u����(STBLib)�̃\�[�X�R�[�h

(2) �T���v���R�[�h���e
  B5T-007001�̊e�ݒ�l�̐ݒ�/�擾�A����сA�ȉ��̊e���������s���A���̌��ʂ�W���o�͂ɏo�͂��Ă��܂��B

   1 : Detection/Estimation                       ��F�؂�����9�@�\�̌��o/��������s���Ă��܂��B
   2 : Recognition(Identify)                      ��F��(����)�����s���Ă��܂��B
   3 : Recognition(Verify)                        ��F��(�ƍ�)�����s���Ă��܂��B
   4 : Register data                              ��F�؃f�[�^�o�^�@�\�����s���Ă��܂��B
   5 : Delete specified data                      �w�肳�ꂽ��F�؃f�[�^�P���폜���Ă��܂��B
   6 : Delete specified user                      �w��̃��[�U�̊�F�؃f�[�^���폜���Ă��܂��B
   7 : Delete all data                            �S�Ẵ��[�U�̊�F�؃f�[�^���폜���Ă��܂��B
   8 : Save Album                                 �A���o�����z�X�g���u���ɕۑ����Ă��܂��B
   9 : Load Album                                 �z�X�g���u���ɕۑ�����Ă���A���o����ǂݍ���ł��܂��B
  10 : Save Album on Flash ROM                    �A���o�����t���b�V��ROM�ɏ�������ł��܂��B
  11 : Reformat Flash ROM                         �t���b�V��ROM�̃A���o���f�[�^�ۑ��̈���ăt�H�[�}�b�g���Ă��܂��B
  12 : Set Number of registered people in album   �A���o���o�^�l���̐ݒ���s���Ă��܂��B
  13 : Get Number of registered people in album   �A���o���o�^�l�����擾���Ă��܂��B

  * �{�T���v���́A���ʁE�N��E��F��(����)�ɑ΂���STBLib��p���邱�Ƃň��艻���������{���Ă��܂��B
    �i�N�����̈�����STBLib�̎g�p�L����I���\�ł�)
  * ��L��3,12,13�̏����́AB5T-007001�̃o�[�W����1.2.3�ȍ~�ł̂ݎ��s�ł��܂��B

(3) �f�B���N�g���\��
    bin/                            �r���h���̏o�̓f�B���N�g��
    import/                         STBLib�𗘗p���邽�߂̃C���|�[�g�f�B���N�g��
    platform/                       �r���h��
        Windows/                        Windows�ł̃r���h��
        Linux/                          Linux�ł̃r���h��
    src/
        HVCApi/                     B5T-007001�C���^�[�t�F�[�X�֐�
            HVCApi.c                    API�֐�
            HVCApi.h                    API�֐���`
            HVCDef.h                    �\���̒�`
            HVCExtraUartFunc.h          API�֐�����Ăяo���O���֐���`
        STBApi/                     STBLib�C���^�[�t�F�[�X�֐�
            STBWrap.c                   STBLib���b�p�[�֐�
            STBWrap.h                   STBLib���b�p�[�֐���`
        bmp/                        B5T-007001����擾�����摜���r�b�g�}�b�v�t�@�C���ɕۑ�����֐�
            bitmap_windows.c            Windows�œ��삷��֐�
            bitmap_linux.c              Linux�œ��삷��֐�
        uart/                       UART�C���^�[�t�F�[�X�֐�
            uart_windows.c              Windows�œ��삷��UART�֐�
            uart_linux.c                Linux�œ��삷��UART�֐�
            uart.h                      UART�֐���`
        Album/                      �A���o���t�@�C���ۑ�/�Ǎ��֐�
            Album.c                     B5T-007001����擾�����A���o����I/O���s���֐�
        Sample/                     �e�����T���v��
            main.c                      �T���v���R�[�h
    STBLib/                         STBLib�֘A�̈ꎮ
        doc/                            STBLib�Ɋւ��鎑���ꎮ
        bin/                            STBLib�r���h����STB.dll�ASTB.lib�o�̓f�B���N�g��
        platform/                       STBLib �r���h��
            Windows/                        Windows�ł̃r���h��
            Linux/                          Linux�ł̃r���h��
        src/                            STBLib �\�[�X�R�[�h�{��

(4) �T���v���R�[�h�̃r���h���@
  * Windows �̏ꍇ
  1. �{�T���v���R�[�h��Windows10/11��œ��삷��悤�쐬���Ă��܂��B
     VC10(Visual Studio 2010 C++)�ȍ~�ŃR���p�C���\�ł��B
  2. �R���p�C����́Abin/Windows�ȉ���exe�t�@�C������������܂��B
     �܂��Aexe�t�@�C���Ɠ����f�B���N�g���ɁASTBLib��DLL�t�@�C�����K�v�ł��B
    �i���炩����STB.dll�͊i�[����Ă��܂��j

  �⑫: STBLib���r���h����ۂ�MFC���C�u�������K�v�ł��B���O�ɃC���X�g�[�����Ă��������B
        STBLib��ύX�����ꍇ�́Asample.exe�t�@�C���Ɠ����ꏊ�ɍŐV��STB.dll��z�u���Ă��������B
        �܂��Aimport/�f�B���N�g���ȉ��̃t�@�C���������ւ��Ă��������B

  * Linux �̏ꍇ
  1. STBLib/platform/Linux�ȉ��ɂ���build.sh���N�����ASTBLib�̃r���h�����{���Ă��������B
  2. STBLib/bin/Linux�z���ɐ������ꂽ�ASTB.a�AlibSTB.so�t�@�C����import/lib�z���փR�s�[���Ă��������B
  3. platform/Linux/Sample�ȉ��ɂ���build.sh�����s���邱�ƂŃR���p�C���A�����N����܂��B

(5) �T���v���R�[�h�̎��s���@
  �{�T���v���R�[�h�̎��s���ɉ��L�̂悤�ɋN���������w�肷��K�v������܂��B

    Usage: sample.exe <com_port> <baudrate> [use_STB]
       com_port: B5T-007001���ڑ����Ă���COM�ԍ�
       baudrate: UART�̃{�[���[�g
       use_STB : STBLib�̎g�p/�s�g�p (STB_ON or STB_OFF)
                 �� ���̈������ȗ������ꍇ�́uSTB_ON�v�Ƃ��ē��삵�܂��B

  ���s��) 
  * Windows�̏ꍇ
   sample.exe 1 921600 STB_ON

  * Linux�̏ꍇ
   - Windows�̏ꍇ�ƋN�������͓��l�ł����A�T���v���R�[�h�����s�����邽�߂ɂ́A
     B5T-007001��/dev/ttyACM0�Ƃ��Đڑ�����Ă��邱�Ƃ��O��ƂȂ�܂��B
     �T���v�����s�p�V�F��(Sample.sh)��p�ӂ��Ă��܂��̂ŁA�Q�l�ɂ��Ă��������B

   - �T���v�����s�p�V�F���ł̋L�ڗ�(Sample.sh:6�s��)
      ./Sample 0 921600 STB_ON
     * ���̏ꍇ�A��1����"0"�ɂ��ẮALinux�łł͖�������܂��B
       "921600bps", "STBLib�g�p"�Ƃ��ċN�����܂��B


[���g�p�ɂ�������]
�E�{�T���v���R�[�h����уh�L�������g�̒��쌠�̓I�������ɋA�����܂��B
�E�{�T���v���R�[�h�͓����ۏ؂�����̂ł͂���܂���B
�E�{�T���v���R�[�h�́AApache License 2.0�ɂĒ񋟂��Ă��܂��B
�ESTBLib��B5T-007001�̐�p�i�ł��B
  ���g�p�ɓ������ẮA�Y�����i�́y�����������z���������̏�ł��g�������������̂Ƃ��܂��B

----
�I�������������
Copyright(C) 2014-2025 OMRON Corporation, All Rights Reserved.
