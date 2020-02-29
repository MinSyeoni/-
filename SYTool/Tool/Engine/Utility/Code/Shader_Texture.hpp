// ��� ���̺�(����) : Ŭ���̾�Ʈ�� �����ϴ� ���� �����͸� ��ǥ�ϴ� �̸�

matrix     g_matWorld;
matrix     g_matView;
matrix     g_matProj;
float g_fChapterX = 0.f;
float g_fChapterY = 0.f;
float g_fX = 1.0f;
float g_fY = 1.0f;

float g_fTime;
//sampler �� : ���� ������ ��� �ִ� �ȷ�Ʈ ������ �ؽ��� ���� ����ü

texture		g_BaseTexture;
sampler  BaseSampler = sampler_state
{
	texture = g_BaseTexture;

minfilter = linear;
magfilter = linear;
};

texture		g_RoundTexture;
sampler  RoundSampler = sampler_state
{
	texture = g_RoundTexture;

minfilter = linear;
magfilter = linear;
};

texture		g_HpTexture;
sampler  HpSampler = sampler_state
{
	texture = g_HpTexture;

minfilter = linear;
magfilter = linear;
};








struct  VS_IN
{
	vector   vPostion : POSITION;			// VECTOR == float4, ������ : �ø�ƽ(���� �Ӽ� ��, fvf, �ݵ�� �빮�ڿ��� ��)
	float2	 vTexUV : TEXCOORD0;
};

struct  VS_OUT
{
	vector   vPostion : POSITION;
	float2	 vTexUV : TEXCOORD0;
};

// ���ؽ� ���̴�
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT  Out = (VS_OUT)0;

	matrix		matWV, matWVP;

	matWV = mul(g_matWorld, g_matView);
	matWVP = mul(matWV, g_matProj);

	Out.vPostion = mul(vector(In.vPostion.xyz, 1.f), matWVP);
	Out.vTexUV = In.vTexUV;

	return Out;
}


// �ȼ� ����ü���� ��� ������ �����ϴ� ���� �ø�ƽ�� ���� POSITION�� ���� ���� ���� ����!

struct PS_IN
{
	// �ؽ��� ������
	float2			vTexUV : TEXCOORD0;
};

struct PS_OUT
{
	vector			vColor : COLOR0;
};

PS_OUT		PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	Out.vColor = tex2D(BaseSampler, In.vTexUV);	// 2���� �ؽ��ķκ��� uv��ǥ�� �ش��ϴ� ���� ���� vector Ÿ������ ��ȯ���ִ� �Լ�

	return Out;
}



technique Default_Device
{
	pass    // ����� ĸ��ȭ//0
	{
		cullmode = none;
		zwriteEnable = true;
		alphablendenable = false;
		srcblend = srcalpha;
		destblend = invsrcalpha;
		vertexshader = compile vs_3_0 VS_MAIN();
		pixelshader = compile ps_3_0 PS_MAIN();
	}
};