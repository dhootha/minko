/*
Copyright (c) 2014 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "minko/Minko.hpp"
#include "minko/MinkoPNG.hpp"
#include "minko/MinkoSDL.hpp"
#include "minko/MinkoBullet.hpp"
#include "minko/MinkoParticles.hpp"
#include "minko/MinkoSerializer.hpp"

std::string MODEL_FILENAME = "lights.scene";

#define DEACTIVATE_PHYSICS

using namespace minko;
using namespace minko::scene;
using namespace minko::component;
using namespace minko::math;

static
std::ostream&
printNodeInfo(std::ostream&, Node::Ptr);

Node::Ptr
createWorldFrame(float axisLength, file::AssetLibrary::Ptr);

void
moveScene(Node::Ptr, float& tx, float& ty, float& tz);

void
toogleParticlesEmittingState(Node::Ptr);

int
main(int argc, char** argv)
{
    auto canvas            = Canvas::create("Minko Example - Scene files");
    auto sceneManager      = SceneManager::create(canvas);
    auto defaultLoader     = sceneManager->assets()->loader();
    auto fxLoader          = file::Loader::create(defaultLoader);

#ifndef DEACTIVATE_PHYSICS
    extension::SerializerExtension::activeExtension<extension::PhysicsExtension>();
#endif //DEACTIVATE_PHYSICS
    extension::SerializerExtension::activeExtension<extension::ParticlesExtension>();

    fxLoader
        ->queue("effect/Phong.effect")
        ->queue("effect/Basic.effect")
        ->queue("effect/Particles.effect");

#ifndef DEACTIVATE_PHYSICS
    fxLoader
        ->queue("effect/Line.effect");
#endif //DEACTIVATE_PHYSICS

    defaultLoader->options()
        ->generateMipmaps(true)
        ->registerParser<file::PNGParser>("png")
        ->registerParser<file::SceneParser>("scene");

    auto fxComplete = fxLoader->complete()->connect([&](file::Loader::Ptr loader)
    {
        defaultLoader->options()->effect(sceneManager->assets()->effect("effect/Phong.effect"));
        defaultLoader->options()->disposeTextureAfterLoading(false);
        defaultLoader->queue(MODEL_FILENAME);
        defaultLoader->load();
    });

    sceneManager->assets()
        ->geometry("cube", geometry::CubeGeometry::create(sceneManager->assets()->context()))
        ->geometry("sphere", geometry::SphereGeometry::create(sceneManager->assets()->context(), 20, 20));

    auto root = scene::Node::create("root")
        ->addComponent(sceneManager);

#ifndef DEACTIVATE_PHYSICS
    auto physicWorld = bullet::PhysicsWorld::create();

    physicWorld->setGravity(math::Vector3::create(0.f, -9.8f, 0.f));

    root->addComponent(physicWorld);
    root->data()->addProvider(canvas->data()); // FIXME
#endif // DEACTIVATE_PHYSICS


    auto camera = scene::Node::create("camera")
        ->addComponent(Renderer::create(0x7f7f7fff))
        ->addComponent(Transform::create(
            Matrix4x4::create()->lookAt(Vector3::zero(), Vector3::create(0.f, 0.f, 20.f))
        ))
        ->addComponent(PerspectiveCamera::create(canvas->aspectRatio()));

    root->addChild(camera);

    auto _ = defaultLoader->complete()->connect([=](file::Loader::Ptr loader)
    {
        auto sceneNode = sceneManager->assets()->symbol(MODEL_FILENAME);
        root->addChild(sceneNode);


        if (!sceneNode->hasComponent<Transform>())
            sceneNode->addComponent(Transform::create());

#if !defined(DEACTIVATE_PHYSICS) && defined(DEBUG)
        auto withColliders = NodeSet::create(sceneNode)
            ->descendants(true)
            ->where([](Node::Ptr n)
            {
                return n->hasComponent<component::bullet::Collider>();
            });

        for (auto& n : withColliders->nodes())
            n->addComponent(bullet::ColliderDebug::create(sceneManager->assets()));
#endif //DEACTIVATE_PHYSICS

        root->addChild(createWorldFrame(5.0f, sceneManager->assets()));
    });

    auto yaw       = float(M_PI) * 0.25f;
    auto pitch     = float(M_PI) * .25f;
    auto roll      = 0.f;
    float minPitch = 0.f + float(1e-5);
    float maxPitch = float(M_PI) - float(1e-5);
    auto lookAt    = Vector3::create(0.f, 0.f, 0.f);
    auto distance  = 25.f;

    Signal<input::Mouse::Ptr, int, int>::Slot mouseMove;
    auto cameraRotationXSpeed = 0.f;
    auto cameraRotationYSpeed = 0.f;

    // handle mouse signals
    auto mouseWheel = canvas->mouse()->wheel()->connect([&](input::Mouse::Ptr m, int h, int v)
    {
        distance += float(v) / 5.f;
    });

    mouseMove = canvas->mouse()->move()->connect([&](input::Mouse::Ptr m, int dx, int dy)
    {
        if (m->leftButtonIsDown())
        {
            cameraRotationYSpeed = float(dx) * .01f;
            cameraRotationXSpeed = float(dy) * -.01f;
        }
    });

    auto resized = canvas->resized()->connect([&](AbstractCanvas::Ptr canvas, uint w, uint h)
    {
        root->children()[0]->component<PerspectiveCamera>()->aspectRatio(float(w) / float(h));
    });

    float tx = 0.0f;
    float ty = 0.0f;
    float tz = 0.0f;
    auto keyDown = canvas->keyboard()->keyDown()->connect([&](input::Keyboard::Ptr k)
    {
        if (k->keyIsDown(input::Keyboard::LEFT))
            tx -= 0.1f;
        else if (k->keyIsDown(input::Keyboard::RIGHT))
            tx += 0.1f;
        else if (k->keyIsDown(input::Keyboard::UP))
            tz += 0.1f;
        else if (k->keyIsDown(input::Keyboard::DOWN))
            tz -= 0.1f;

        // for particles
        if (k->keyIsDown(input::Keyboard::SPACE))
            toogleParticlesEmittingState(root);
    });

    auto enterFrame = canvas->enterFrame()->connect([&](AbstractCanvas::Ptr canvas, float time, float deltaTime)
    {
        yaw += cameraRotationYSpeed;
        cameraRotationYSpeed *= 0.9f;

        pitch += cameraRotationXSpeed;
        cameraRotationXSpeed *= 0.9f;

        if (pitch > maxPitch)
            pitch = maxPitch;
        else if (pitch < minPitch)
            pitch = minPitch;

        camera->component<Transform>()->matrix()->lookAt(
            lookAt,
            Vector3::create(
                lookAt->x() + distance * std::cos(yaw) * std::sin(pitch),
                lookAt->y() + distance * std::cos(pitch),
                lookAt->z() + distance * std::sin(yaw) * std::sin(pitch)
            )
        );

        // change the position of the serialized scene
        moveScene(sceneManager->assets()->symbol(MODEL_FILENAME), tx, ty, tz);

        sceneManager->nextFrame(time, deltaTime);
    });

    fxLoader->load();
    canvas->run();
}

void
toogleParticlesEmittingState(Node::Ptr root)
{
    if (root == nullptr)
        return;

    auto withParticles = NodeSet::create(root)
        ->descendants(true)
        ->where([](Node::Ptr n)
        {
            return n->hasComponent<component::ParticleSystem>();
        });

    for (auto& n : withParticles->nodes())
    {
        auto particles = n->component<component::ParticleSystem>();

        particles->emitting(!particles->emitting());
    }
}

void
moveScene(Node::Ptr model, float& tx, float& ty, float& tz)
{
    if (model && model->hasComponent<Transform>())
    {
        model->component<Transform>()->matrix()->appendTranslation(tx, ty, tz);

#if !defined(DEACTIVATE_PHYSICS) && defined(DEBUG)
        auto withColliders = scene::NodeSet::create(model)
            ->descendants(true)
            ->where([](scene::Node::Ptr n)
            {
                return n->hasComponent<bullet::Collider>();
            });

        for (auto& n : withColliders->nodes())
            n->component<bullet::Collider>()->synchronizePhysicsWithGraphics();
#endif //DEACTIVATE_PHYSICS
    }

    tx = 0.0f;
    ty = 0.0f;
    tz = 0.0f;
}

Node::Ptr
createWorldFrame(float axisLength, file::AssetLibrary::Ptr assets)
{
    assert(assets->geometry("cube") && assets->effect("basic"));

    auto xAxis = Node::create("x-axis")
        ->addComponent(Surface::create(
            assets->geometry("cube"),
            material::BasicMaterial::create()->diffuseColor(0xff0000ff),
            assets->effect("basic")
        ))
        ->addComponent(Transform::create(
            Matrix4x4::create()
                ->appendTranslation(0.5f, 0.0f, 0.0f)
                ->appendScale(axisLength, 0.01f, 0.01f)
        ));

    auto yAxis = Node::create("y-axis")
        ->addComponent(Surface::create(
            assets->geometry("cube"),
            material::BasicMaterial::create()->diffuseColor(0x00ff00ff),
            assets->effect("basic")
        ))
        ->addComponent(Transform::create(
            Matrix4x4::create()
                ->appendTranslation(0.0f, 0.5f, 0.0f)
                ->appendScale(0.01f, axisLength, 0.01f)
        ));

    auto zAxis = Node::create("z-axis")
        ->addComponent(Surface::create(
            assets->geometry("cube"),
            material::BasicMaterial::create()->diffuseColor(0x0000ffff),
            assets->effect("basic")
        ))
        ->addComponent(Transform::create(
            Matrix4x4::create()
                ->appendTranslation(0.0f, 0.0f, 0.5f)
                ->appendScale(0.01f, 0.01f, axisLength)
        ));

    return Node::create("frame")
        ->addChild(xAxis)
        ->addChild(yAxis)
        ->addChild(zAxis);
}

static
std::ostream&
printNodeInfo(std::ostream&    out,
              Node::Ptr        node)
{
    if (node == nullptr)
        return out;

    out << "\n'" << node->name() << "'" << (node->parent() ? "\t<- '" + node->parent()->name() + "'" : "") << std::endl;

    if (node->hasComponent<Transform>())
    {
        const auto& m = node->component<Transform>()->matrix()->data();

        out << "\t[" << m[0] << " " << m[1] << " " << m[2] << ";\n\t"
            << m[4] << " " << m[5] << " " << m[6] << ";\n\t"
            << m[8] << " " << m[9] << " " << m[10] << "]\n\t"
            << "t = (" << m[3] << ", " << m[7] << ", " << m[11] << ")"
            << std::endl;
    }

    for (auto& n : node->children())
        printNodeInfo(out, n);

    return out;
}
