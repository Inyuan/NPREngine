Texture2D profileData: register(t0);
RWTexture2D<float4> g_Output : register(u0);


[numthreads(32, 32, 1)]
void CS(uint3 DTid : SV_DispatchThreadID)
{
    float outcolor = profileData[DTid.xy].r;
    float thick = profileData[DTid.xy].z / 2.0f;
    
    /*
    -1 //0  //1
      1// 1 // 1
    /////////////
    -1 //0  //1
      0// 0 // 0
    /////////////
    -1 //0  //1
     -1// -1// -1
    */
    uint2 position_1 = DTid.xy;
    position_1.x++;
    uint2 position_2 = DTid.xy;
    position_2.x--;
    uint2 position_3 = DTid.xy;
    position_3.y++;
    uint2 position_4 = DTid.xy;
    position_4.y--;
    uint2 position_5 = DTid.xy;
    position_5.x++;//(1,1)
    position_5.y++;
    uint2 position_6 = DTid.xy;
    position_6.x++;//(1,-1)
    position_6.y--;
    uint2 position_7 = DTid.xy;
    position_7.y++;//(-1,1)
    position_7.x--;
    uint2 position_8 = DTid.xy;
    position_8.y--;//(-1,-1)
    position_8.x--;

    
    float outcolor_1 = profileData[position_1].r;
    float outcolor_2 = profileData[position_2].r;
    float outcolor_3 = profileData[position_3].r;
    float outcolor_4 = profileData[position_4].r;
    float outcolor_5 = profileData[position_5].r;
    float outcolor_6 = profileData[position_6].r;
    float outcolor_7 = profileData[position_7].r;
    float outcolor_8 = profileData[position_8].r;
    //0Îª±ß
    
    outcolor += outcolor_1;
    outcolor += outcolor_2;
    outcolor += outcolor_3;
    outcolor += outcolor_4;
    outcolor += outcolor_5;
    outcolor += outcolor_6;
    outcolor += outcolor_7;
    outcolor += outcolor_8;
    outcolor /= 2.0f;

    g_Output[DTid.xy] = float4(1.0f, 1.0f, 1.0f, 1.0f);
    if (outcolor >= 1.0f
        && thick > 0.0f)
    {
        position_1 = DTid.xy;
        position_2 = DTid.xy;
        position_3 = DTid.xy;
        position_4 = DTid.xy;
        float4 result = float4(0.0f, 1.0f, 1.0f, 1.0f);
        float j = 0.01f;
        float k = 0.01f;
        g_Output[DTid.xy] = result;
        float thick_buf = thick - 0.01f;

        //coop
        for (j = 0; j < thick; j += 0.01f)
        {
            position_1.y++;
            position_1.x = DTid.x;
            position_2.x++;
            position_2.y = DTid.y;
            position_3.y--;
            position_3.x = DTid.x;
            position_4.x--;
            position_4.y = DTid.y;

            thick_buf -= j;
            for (k = 0; k < thick_buf; k += 0.01f)
            {
                g_Output[position_1] = result;
                g_Output[position_2] = result;
                g_Output[position_3] = result;
                g_Output[position_4] = result; 
                position_1.x--;
                position_2.y++;
                position_3.x++;
                position_4.y--;
            }
        }
    }
}

/*
* 0 0 0 1 0 0 0
* 0 0 1 1 1 0 0
* 0 1 1 1 1 1 0
* 1 1 1 1 1 1 1
* 0 1 1 1 1 1 0
* 0 0 1 1 1 0 0
* 0 0 0 1 0 0 0
*/