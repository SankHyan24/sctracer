


vec3 PhongEval(State state, vec3 V, vec3 N, vec3 L, out float pdf);

vec3 EvalPhongDiffuse(Material mat, vec3 Csheen, vec3 V, vec3 L, vec3 H, out float pdf)


vec3 PhongEval(State state, vec3 V, vec3 N, vec3 L, out float pdf){
    pdf = 0.0;
    vec3 f = vec3(0.0);
    vec3 T, B;
    Onb(N, T, B);

    V = ToLocal(T, B, N, V);
    L = ToLocal(T, B, N, L);

    vec3 H;
    if (L.z > 0.0)
        H = normalize(L + V);
    else
        H = normalize(L + V * state.eta);

    if (H.z < 0.0)
        H = -H;

    // weight count
    MatPhong mat = MaterialToPhong(state.mat);
    float kd = Luminance(mat.Kd);
    float ks = mat.metallic;
    float wd = kd / (kd + ks);
    float ws = ks / (kd + ks);



}
