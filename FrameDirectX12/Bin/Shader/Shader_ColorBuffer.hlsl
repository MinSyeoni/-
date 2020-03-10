
/*____________________________________________________________________
- register�� ������ۿ� ���ҵ��� ����ϴ½ø�ƽ�Դϴ�. ���ڴ� �ɼ��Դϴ�.
b - �������
t - �ؽ�ó
s - ���÷�

- packoffset�� �� ���� �ε��Ҽ��� 4���� ó���� �� �ִ� GPU�� ����ؼ� ��������� �ɹ����� ���� �ɼ��Դϴ�.
 float3 �� float �� �� �ɼ����� ���� �� �ֽ��ϴ�.
______________________________________________________________________*/
cbuffer cbMatrixInfo	: register(b0)
{
    float4x4 matWorld : packoffset(c0);
    float4x4 matView : packoffset(c4);
    float4x4 matProj : packoffset(c8);
    float4x4 matWVP : packoffset(c12);
};

struct VS_IN
{
	float3 vPos		: POSITION;
	float4 vColor	: COLOR;
};

struct VS_OUT
{
	float4 vPos		: SV_POSITION;
	float4 vColor	: COLOR;
};

VS_OUT VS_MAIN(VS_IN Input)
{
	VS_OUT Output= (VS_OUT)0 ;
	
	Output.vPos		= mul(float4(Input.vPos, 1.0f), matWVP);
	Output.vColor	= Input.vColor;
	
	return (Output);
}

float4 PS_MAIN(VS_OUT Input) : SV_TARGET
{
	return (Input.vColor);
}

