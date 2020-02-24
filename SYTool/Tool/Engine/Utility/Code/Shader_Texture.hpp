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

VS_OUT VS_MAIN_ANI(VS_IN In)
{
	VS_OUT  Out = (VS_OUT)0;

	matrix		matWV, matWVP;

	matWV = mul(g_matWorld, g_matView);
	matWVP = mul(matWV, g_matProj);

	Out.vPostion = mul(vector(In.vPostion.xyz, 1.f), matWVP);

	Out.vTexUV.x = (In.vTexUV.x * g_fChapterX) + g_fX * g_fChapterX;
	Out.vTexUV.y = (In.vTexUV.y * g_fChapterY) + g_fY * g_fChapterY;


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
	vector          vNormal : COLOR1;
	vector          vDepth : COLOR2;
	vector          vLIm : COLOR3;
};

PS_OUT		PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	Out.vColor = tex2D(BaseSampler, In.vTexUV);	// 2���� �ؽ��ķκ��� uv��ǥ�� �ش��ϴ� ���� ���� vector Ÿ������ ��ȯ���ִ� �Լ�

	return Out;
}
PS_OUT		PS_ALPHAMAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	vector vColor = tex2D(BaseSampler, In.vTexUV);	// 2���� �ؽ��ķκ��� uv��ǥ�� �ش��ϴ� ���� ���� vector Ÿ������ ��ȯ���ִ� �Լ�
	Out.vColor = vColor;
	Out.vColor.a = 0.5f;
	return Out;
}

PS_OUT		PS_LIGHT(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	Out.vColor = tex2D(BaseSampler, In.vTexUV);	// 2���� �ؽ��ķκ��� uv��ǥ�� �ش��ϴ� ���� ���� vector Ÿ������ ��ȯ���ִ� �Լ�
	Out.vLIm = Out.vColor;
	return Out;
}


PS_OUT  PS_DESERVE(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	Out.vColor = tex2D(BaseSampler, In.vTexUV);

	Out.vColor *= (1 - g_fTime);

	return Out;
}
PS_OUT PS_HP(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	Out.vColor = tex2D(BaseSampler, In.vTexUV);
	vector vRound = tex2D(RoundSampler, In.vTexUV);



	Out.vColor *= vRound;

	return Out;



}
PS_OUT PS_HPBAR(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	vector vColor = tex2D(BaseSampler, In.vTexUV);
	vector vColor2 = tex2D(HpSampler, In.vTexUV);
	Out.vColor = vector(0.0f, 1.0f, 0.0f, 1.0f);

	Out.vColor *= vColor;
	Out.vColor *= vColor2;

	

	return Out;



}
PS_OUT PS_INDI(PS_IN In)
{

	PS_OUT			Out = (PS_OUT)0;

	vector vColor = tex2D(BaseSampler, In.vTexUV);

	Out.vColor= vColor;
	Out.vColor = vColor.r*vector(0.2f,0.2f,1.0f,1.0f);
	

	return Out;



}

PS_OUT PS_INDI2(PS_IN In)
{

	PS_OUT			Out = (PS_OUT)0;

	vector vColor = tex2D(BaseSampler, In.vTexUV);

	Out.vColor = vColor;
	Out.vColor = vColor.g*vector(0.2f, 0.2f, 1.0f, 1.0f);


	return Out;



}
PS_OUT PS_TRAIL(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	vector vColor = tex2D(BaseSampler, In.vTexUV);

	Out.vColor = vColor;
	Out.vColor = vColor.b*vector(1.0f, 0.6f, 0.2f, 1.0f);
	/*Out.vColor = Out.vColor*(3.f-g_fTime);*/
	return Out;
}








technique Default_Device
{
	pass    // ����� ĸ��ȭ//0
{
	cullmode = none;
    zwriteEnable =true ;
	alphablendenable = true;
	srcblend = srcalpha;
	destblend = invsrcalpha;
    vertexshader = compile vs_3_0 VS_MAIN();
    pixelshader = compile ps_3_0 PS_MAIN();
}
pass//1
{
	cullmode = none;
alphablendenable = true;
srcblend = srcalpha;
destblend = invsrcalpha;
zwriteEnable = true;
vertexshader = compile vs_3_0 VS_MAIN();
pixelshader = compile ps_3_0 PS_ALPHAMAIN();

}
pass //2
{

cullmode = none;
alphablendenable = true;
vertexshader = compile vs_3_0 VS_MAIN();
pixelshader = compile ps_3_0 PS_DESERVE();

}
pass//3
{
	cullmode = none;
alphatestenable = true;
alphafunc = Greater;
alpharef = 0xc0;
vertexshader = compile vs_3_0 VS_MAIN();
pixelshader = compile ps_3_0 PS_LIGHT();

}
pass//4
{
	alphatestenable = true;
alphafunc = Greater;
alpharef = 0xc0;

  vertexshader = compile vs_3_0 VS_MAIN();
  pixelshader = compile ps_3_0 PS_HP();

}
pass//5
{
	alphatestenable = true;
alphafunc = Greater;
alpharef = 0xc0;

vertexshader = compile vs_3_0 VS_MAIN();
pixelshader = compile ps_3_0 PS_HPBAR();

}

pass//6
{
	cullmode = none;
alphablendenable = true;
srcblend = srcalpha;
destblend = invsrcalpha;

vertexshader = compile vs_3_0 VS_MAIN();
pixelshader = compile ps_3_0 PS_INDI();

}

pass//7
{
	cullmode = none;
alphablendenable = true;
srcblend = srcalpha;
destblend = invsrcalpha;

vertexshader = compile vs_3_0 VS_MAIN();
pixelshader = compile ps_3_0 PS_INDI2();

}
pass //8
{
	/*alphatestenable = true;
alphafunc = Greater;
alpharef = 0xc0;*/
	cullmode = none;
alphablendenable = true;
srcblend = srcalpha;
destblend = invsrcalpha;

vertexshader = compile vs_3_0 VS_MAIN();
pixelshader = compile ps_3_0 PS_TRAIL();

}

pass//9
{
	cullmode = none;
	zwriteEnable = true;
	alphablendenable = true;
	srcblend = srcalpha;
	destblend = invsrcalpha;
	vertexshader = compile vs_3_0 VS_MAIN_ANI();
	pixelshader = compile ps_3_0 PS_LIGHT();
}

};