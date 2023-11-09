#define WIN32_LEAN_AND_MEAN 
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <windowsx.h>
#include "glad/glad.h"
#include <cmath>
#include "GeometryUtils.h"
#include "GraphicsPrimitives.h"

#ifndef  M_PI
#define M_PI 3.14159265358979323846f
#endif

void RenderMesh(const ModelData& model) {
    glPushMatrix();

    mat4 worldMatrix = GetWorldTransform(model);
    glMultMatrixf(worldMatrix.asArray);

    if (model.GetMesh() != 0) {
        RenderMesh(model.GetMesh());
    }

    glPopMatrix();
}

void RenderMesh(const MeshData& mesh) {
    glBegin(GL_TRIANGLES);

    for (int i = 0; i < mesh.numTriangles; ++i) {
        vec3 triangleNormal = ComputeTriangleNormal(mesh.triangles[i]);
        glNormal3fv(triangleNormal.asArray);

        glVertex3fv(mesh.triangles[i].a.asArray);
        glVertex3fv(mesh.triangles[i].b.asArray);
        glVertex3fv(mesh.triangles[i].c.asArray);
    }

    glEnd();
}

void RenderCollisionManifold(const CollisionData& collision) {
    if (!collision.colliding) {
        return;
    }

    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_POINTS);

    for (int i = 0; i < collision.contacts.size(); ++i) {
        glVertex3f(collision.contacts[i].x, collision.contacts[i].y, collision.contacts[i].z);
    }

    glEnd();

    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    vec3 center = vec3();

    for (int i = 0; i < collision.contacts.size(); ++i) {
        vec3 startPoint = collision.contacts[i];
        vec3 endPoint = startPoint + collision.normal * collision.penetration;
        center = center + startPoint;

        glVertex3fv(startPoint.asArray);
        glVertex3fv(endPoint.asArray);
    }

    glEnd();

    if (collision.contacts.size() == 0) {
        return;
    }

    float invContactsSize = 1.0f / (float)collision.contacts.size();
    center = center * invContactsSize;

    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    vec3 startPoint = center;
    vec3 endPoint = center + collision.normal;

    glVertex3fv(startPoint.asArray);
    glVertex3fv(endPoint.asArray);

    glEnd();
}

void RenderNormals(const FrustumData& frustum) {
    vec3 nearTopLeft = Intersection(frustum.nearPlane, frustum.top, frustum.left);
    vec3 nearTopRight = Intersection(frustum.nearPlane, frustum.top, frustum.right);
    vec3 nearBottomLeft = Intersection(frustum.nearPlane, frustum.bottom, frustum.left);
    vec3 nearBottomRight = Intersection(frustum.nearPlane, frustum.bottom, frustum.right);

    vec3 farTopLeft = Intersection(frustum.farPlane, frustum.top, frustum.left);
    vec3 farTopRight = Intersection(frustum.farPlane, frustum.top, frustum.right);
    vec3 farBottomLeft = Intersection(frustum.farPlane, frustum.bottom, frustum.left);
    vec3 farBottomRight = Intersection(frustum.farPlane, frustum.bottom, frustum.right);

    vec3 centers[] = {
        (nearTopRight + nearTopLeft + farTopRight + farTopLeft) * 0.25f, // Top
        (nearBottomRight + nearBottomLeft + farBottomRight + farBottomLeft) * 0.25f, // Bottom
        (nearTopLeft + nearBottomLeft + farTopLeft + farBottomLeft) * 0.25f, // Left
        (nearTopRight + nearBottomRight + farTopRight + farBottomRight) * 0.25f, // Right
        (nearTopLeft + nearTopRight + nearBottomLeft + nearBottomRight) * 0.25f, // Near
        (farTopLeft + farTopRight + farBottomLeft + farBottomRight) * 0.25f // Far
    };

    glBegin(GL_LINES);
    for (int i = 0; i < 6; ++i) {
        vec3 p1 = centers[i] + frustum.planes[i].normal * 0.5f;
        glVertex3fv(centers[i].asArray);
        glVertex3fv(p1.asArray);
    }

    glEnd();

    for (int i = 0; i < 6; ++i) {
        vec3 p1 = centers[i] + frustum.planes[i].normal * 0.5f;
        DebugRenderPlane(frustum.planes[i], p1);
    }
}

void RenderAABB(const AABB& aabb) {
    vec3 min = aabb.GetMin();
    vec3 max = aabb.GetMax();
    vec3 corners[8];

    corners[0] = vec3(min.x, min.y, min.z);
    corners[1] = vec3(max.x, min.y, min.z);
    corners[2] = vec3(max.x, max.y, min.z);
    corners[3] = vec3(min.x, max.y, min.z);
    corners[4] = vec3(min.x, min.y, max.z);
    corners[5] = vec3(max.x, min.y, max.z);
    corners[6] = vec3(max.x, max.y, max.z);
    corners[7] = vec3(min.x, max.y, max.z);

    int indices[24] = {
        0, 1, 1, 2, 2, 3, 3, 0, // Bottom
        4, 5, 5, 6, 6, 7, 7, 4, // Top
        0, 4, 1, 5, 2, 6, 3, 7  // Sides
    };

    glBegin(GL_LINES);
    for (int i = 0; i < 24; i += 2) {
        glVertex3fv(corners[indices[i]].asArray);
        glVertex3fv(corners[indices[i + 1]].asArray);
    }
    glEnd();
}

void RenderSphere(const Sphere& sphere) {
    const int numSegments = 32;
    const int numRings = 16;
    const float deltaTheta = 2.0f * M_PI / numSegments;
    const float deltaPhi = M_PI / numRings;

    glBegin(GL_TRIANGLES);

    for (int i = 0; i < numRings; ++i) {
        for (int j = 0; j < numSegments; ++j) {
            float theta1 = j * deltaTheta;
            float theta2 = (j + 1) * deltaTheta;
            float phi1 = i * deltaPhi;
            float phi2 = (i + 1) * deltaPhi;

            vec3 p1 = sphere.GetPoint(theta1, phi1);
            vec3 p2 = sphere.GetPoint(theta2, phi1);
            vec3 p3 = sphere.GetPoint(theta1, phi2);
            vec3 p4 = sphere.GetPoint(theta2, phi2);

            // Triangle 1
            glNormal3fv(p1.asArray);
            glVertex3fv(p1.asArray);
            glNormal3fv(p2.asArray);
            glVertex3fv(p2.asArray);
            glNormal3fv(p4.asArray);
            glVertex3fv(p4.asArray);

            // Triangle 2
            glNormal3fv(p1.asArray);
            glVertex3fv(p1.asArray);
            glNormal3fv(p4.asArray);
            glVertex3fv(p4.asArray);
            glNormal3fv(p3.asArray);
            glVertex3fv(p3.asArray);
        }
    }

    glEnd();
}