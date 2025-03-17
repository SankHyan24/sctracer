#include <cpu/integrator.hpp>

namespace scTracer::CPU
{
    glm::vec3 Integrator::ToWorld(glm::vec3 X, glm::vec3 Y, glm::vec3 Z, glm::vec3 V)
    {
        return V.x * X + V.y * Y + V.z * Z;
    }

    glm::vec3 Integrator::ToLocal(glm::vec3 X, glm::vec3 Y, glm::vec3 Z, glm::vec3 V)
    {
        return glm::vec3(glm::dot(V, X), glm::dot(V, Y), glm::dot(V, Z));
    }

    void Integrator::TintColors(Material mat, float eta, float F0, glm::vec3 &Csheen, glm::vec3 &Cspec0)
    {
        float lum = Luminance(mat.baseColor);
        glm::vec3 ctint = lum > 0.0 ? mat.baseColor / lum : glm::vec3(1.0);

        F0 = (1.0 - eta) / (1.0 + eta);
        F0 *= F0;

        Cspec0 = F0 * glm::mix(glm::vec3(1.0), ctint, mat.specularTint);
        Csheen = glm::mix(glm::vec3(1.0), ctint, mat.sheenTint);
    }

    glm::vec3 Integrator::EvalDisneyDiffuse(Material mat, glm::vec3 Csheen, glm::vec3 V, glm::vec3 L, glm::vec3 H, float &pdf)
    {
        pdf = 0.0;
        if (L.z <= 0.0)
            return glm::vec3(0.0);

        float LDotH = glm::dot(L, H);

        float Rr = 2.0 * mat.roughness * LDotH * LDotH;

        // Diffuse
        float FL = SchlickWeight(L.z);
        float FV = SchlickWeight(V.z);
        float Fretro = Rr * (FL + FV + FL * FV * (Rr - 1.0));
        float Fd = (1.0 - 0.5 * FL) * (1.0 - 0.5 * FV);

        // Fake subsurface
        float Fss90 = 0.5 * Rr;
        float Fss = glm::mix(float(1.0), Fss90, FL) * glm::mix(float(1.0), Fss90, FV);
        float ss = 1.25 * (Fss * (1.0 / (L.z + V.z) - 0.5) + 0.5);

        // Sheen
        float FH = SchlickWeight(LDotH);
        glm::vec3 Fsheen = FH * mat.sheen * Csheen;

        pdf = L.z * INV_PI;
        return float(INV_PI) * mat.baseColor * glm::mix(Fd + Fretro, ss, mat.subsurface) + Fsheen;
    }

    glm::vec3 Integrator::EvalMicrofacetReflection(Material mat, glm::vec3 V, glm::vec3 L, glm::vec3 H, glm::vec3 F, float &pdf)
    {
        pdf = 0.0;
        if (L.z <= 0.0)
            return glm::vec3(0.0);

        float D = GTR2Aniso(H.z, H.x, H.y, mat.ax, mat.ay);
        float G1 = SmithGAniso(abs(V.z), V.x, V.y, mat.ax, mat.ay);
        float G2 = G1 * SmithGAniso(abs(L.z), L.x, L.y, mat.ax, mat.ay);

        pdf = G1 * D / (4.0f * V.z);
        return F * D * G2 / (4.0f * L.z * V.z);
    }

    glm::vec3 Integrator::DisneyEval(State state, glm::vec3 V, glm::vec3 N, glm::vec3 L, float &pdf)
    {
        pdf = 0.0;
        glm::vec3 f = glm::vec3(0.0);

        // TODO: Tangent and bitangent should be calculated from mesh (provided, the mesh has proper uvs)
        glm::vec3 T(0.0f), B(0.0f);
        Onb(N, T, B);

        // Transform to shading space to simplify operations (NDotL = L.z; NDotV = V.z; NDotH = H.z)
        V = ToLocal(T, B, N, V);
        L = ToLocal(T, B, N, L);

        glm::vec3 H;
        if (L.z > 0.0)
            H = glm::normalize(L + V);
        else
            H = glm::normalize(L + V * state.eta);

        if (H.z < 0.0)
            H = -H;

        // Tint colors
        glm::vec3 Csheen, Cspec0;
        float F0(0.0);
        TintColors(state.mat, state.eta, F0, Csheen, Cspec0);

        // Model weights
        float dielectricWt = (1.0 - state.mat.metallic) * (1.0 - state.mat.specTrans);
        float metalWt = state.mat.metallic;
        float glassWt = (1.0 - state.mat.metallic) * state.mat.specTrans;

        // Lobe probabilities
        float schlickWt = SchlickWeight(V.z);

        float diffPr = dielectricWt * Luminance(state.mat.baseColor);
        float dielectricPr = dielectricWt * Luminance(glm::mix(Cspec0, glm::vec3(1.0), schlickWt));
        float metalPr = metalWt * Luminance(glm::mix(state.mat.baseColor, glm::vec3(1.0), schlickWt));
        float glassPr = glassWt;
        float clearCtPr = 0.25 * state.mat.clearcoat;

        // Normalize probabilities
        float invTotalWt = 1.0 / (diffPr + dielectricPr + metalPr + glassPr + clearCtPr);
        diffPr *= invTotalWt;
        dielectricPr *= invTotalWt;
        metalPr *= invTotalWt;
        glassPr *= invTotalWt;
        clearCtPr *= invTotalWt;

        bool reflect = L.z * V.z > 0;

        float tmpPdf = 0.0;
        float VDotH = abs(glm::dot(V, H));

        // Diffuse
        if (diffPr > 0.0 && reflect)
        {
            f += EvalDisneyDiffuse(state.mat, Csheen, V, L, H, tmpPdf) * dielectricWt;
            pdf += tmpPdf * diffPr;
        }

        // Dielectric Reflection
        if (dielectricPr > 0.0 && reflect)
        {
            // Normalize for interpolating based on Cspec0
            float F = (DielectricFresnel(VDotH, 1.0 / state.mat.ior) - F0) / (1.0 - F0);

            f += EvalMicrofacetReflection(state.mat, V, L, H, glm::mix(Cspec0, glm::vec3(1.0f), F), tmpPdf) * dielectricWt;
            pdf += tmpPdf * dielectricPr;
        }

        // Metallic Reflection
        if (metalPr > 0.0 && reflect)
        {
            // Tinted to base color
            glm::vec3 F = glm::mix(state.mat.baseColor, glm::vec3(1.0), SchlickWeight(VDotH));

            f += EvalMicrofacetReflection(state.mat, V, L, H, F, tmpPdf) * metalWt;
            pdf += tmpPdf * metalPr;
        }

        // Glass/Specular BSDF
        if (glassPr > 0.0)
        {
            // Dielectric fresnel (achromatic)
            float F = DielectricFresnel(VDotH, state.eta);

            if (reflect)
            {
                f += EvalMicrofacetReflection(state.mat, V, L, H, glm::vec3(F), tmpPdf) * glassWt;
                pdf += tmpPdf * glassPr * F;
            }
            else
            {
                f += EvalMicrofacetRefraction(state.mat, state.eta, V, L, H, glm::vec3(F), tmpPdf) * glassWt;
                pdf += tmpPdf * glassPr * (1.0 - F);
            }
        }

        // Clearcoat
        if (clearCtPr > 0.0 && reflect)
        {
            f += EvalClearcoat(state.mat, V, L, H, tmpPdf) * 0.25f * state.mat.clearcoat;
            pdf += tmpPdf * clearCtPr;
        }

        return f * abs(L.z);
    }

    glm::vec3 Integrator::DisneySample(State state, glm::vec3 V, glm::vec3 N, glm::vec3 &L, float &pdf)
    {
        pdf = 0.0;

        float r1 = rand();
        float r2 = rand();

        // TODO: Tangent and bitangent should be calculated from mesh (provided, the mesh has proper uvs)
        glm::vec3 T(0.0f), B(0.0f);
        Onb(N, T, B);

        // Transform to shading space to simplify operations (NDotL = L.z; NDotV = V.z; NDotH = H.z)
        V = ToLocal(T, B, N, V);

        // Tint colors
        glm::vec3 Csheen, Cspec0;
        float F0(0.0f);
        TintColors(state.mat, state.eta, F0, Csheen, Cspec0);

        // Model weights
        float dielectricWt = (1.0 - state.mat.metallic) * (1.0 - state.mat.specTrans);
        float metalWt = state.mat.metallic;
        float glassWt = (1.0 - state.mat.metallic) * state.mat.specTrans;

        // Lobe probabilities
        float schlickWt = SchlickWeight(V.z);

        float diffPr = dielectricWt * Luminance(state.mat.baseColor);
        float dielectricPr = dielectricWt * Luminance(glm::mix(Cspec0, glm::vec3(1.0), schlickWt));
        float metalPr = metalWt * Luminance(glm::mix(state.mat.baseColor, glm::vec3(1.0), schlickWt));
        float glassPr = glassWt;
        float clearCtPr = 0.25 * state.mat.clearcoat;

        // Normalize probabilities
        float invTotalWt = 1.0 / (diffPr + dielectricPr + metalPr + glassPr + clearCtPr);
        diffPr *= invTotalWt;
        dielectricPr *= invTotalWt;
        metalPr *= invTotalWt;
        glassPr *= invTotalWt;
        clearCtPr *= invTotalWt;

        // CDF of the sampling probabilities
        float cdf[5];
        cdf[0] = diffPr;
        cdf[1] = cdf[0] + dielectricPr;
        cdf[2] = cdf[1] + metalPr;
        cdf[3] = cdf[2] + glassPr;
        cdf[4] = cdf[3] + clearCtPr;

        // Sample a lobe based on its importance
        float r3 = rand();

        if (r3 < cdf[0]) // Diffuse
        {
            L = CosineSampleHemisphere(r1, r2);
        }
        else if (r3 < cdf[2]) // Dielectric + Metallic reflection
        {
            glm::vec3 H = SampleGGXVNDF(V, state.mat.ax, state.mat.ay, r1, r2);

            if (H.z < 0.0)
                H = -H;

            L = glm::normalize(reflect(-V, H));
        }
        else if (r3 < cdf[3]) // Glass
        {
            glm::vec3 H = SampleGGXVNDF(V, state.mat.ax, state.mat.ay, r1, r2);
            float F = DielectricFresnel(abs(glm::dot(V, H)), state.eta);

            if (H.z < 0.0)
                H = -H;

            // Rescale random number for reuse
            r3 = (r3 - cdf[2]) / (cdf[3] - cdf[2]);

            // Reflection
            if (r3 < F)
            {
                L = glm::normalize(reflect(-V, H));
            }
            else // Transmission
            {
                L = glm::normalize(refract(-V, H, state.eta));
            }
        }
        else // Clearcoat
        {
            glm::vec3 H = SampleGTR1(state.mat.clearcoatRoughness, r1, r2);

            if (H.z < 0.0)
                H = -H;

            L = normalize(reflect(-V, H));
        }

        L = ToWorld(T, B, N, L);
        V = ToWorld(T, B, N, V);

        return DisneyEval(state, V, N, L, pdf);
    }

    glm::vec3 Integrator::EvalMicrofacetRefraction(Material mat, float eta, glm::vec3 V, glm::vec3 L, glm::vec3 H, glm::vec3 F, float &pdf)
    {
        pdf = 0.0;
        if (L.z >= 0.0)
            return glm::vec3(0.0);

        float LDotH = dot(L, H);
        float VDotH = dot(V, H);

        float D = GTR2Aniso(H.z, H.x, H.y, mat.ax, mat.ay);
        float G1 = SmithGAniso(abs(V.z), V.x, V.y, mat.ax, mat.ay);
        float G2 = G1 * SmithGAniso(abs(L.z), L.x, L.y, mat.ax, mat.ay);
        float denom = LDotH + VDotH * eta;
        denom *= denom;
        float eta2 = eta * eta;
        float jacobian = abs(LDotH) / denom;

        pdf = G1 * glm::max(0.0f, VDotH) * D * jacobian / V.z;
        return glm::pow(mat.baseColor, glm::vec3(0.5f)) * (1.0f - F) * D * G2 * abs(VDotH) * jacobian * eta2 / abs(L.z * V.z);
    }

    glm::vec3 Integrator::EvalClearcoat(Material mat, glm::vec3 V, glm::vec3 L, glm::vec3 H, float &pdf)
    {
        pdf = 0.0;
        if (L.z <= 0.0)
            return glm::vec3(0.0);

        float VDotH = glm::dot(V, H);

        float F = glm::mix(0.04f, 1.0f, SchlickWeight(VDotH));
        float D = GTR1(H.z, mat.clearcoatRoughness);
        float G = SmithG(L.z, 0.25) * SmithG(V.z, 0.25);
        float jacobian = 1.0 / (4.0 * VDotH);

        pdf = D * H.z * jacobian;
        return glm::vec3(F) * D * G;
    }
}