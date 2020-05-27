#include "ofxContentsManager.h"

static const string MODULE_NAME = "ofxContentsManager";

namespace ofxContentsManager
{
    
    //---------------------------------------------------------------------------------------
    /*
     BASE CONTENT CLASS
     */
    //---------------------------------------------------------------------------------------
    Content::Content() {
        bufferWidth = 0;
        bufferHeight = 0;
    }
    
    void Content::initGUI(int posGuiConent, int widthContentGui) {
        this->widthContentGui = widthContentGui;
        this->posGuiConent    = posGuiConent;
        
        pathSettings = "settings_" + getName() + ".json";
        gui.setDefaultWidth(widthContentGui);
        gui.setup(ofToUpper(getName()), pathSettings);
        gui.setPosition(posGuiConent, 0);
        
        isGUIEnabled = true;
    }
    
    void Content::loadGUISettings() {
        gui.loadFromFile(pathSettings);
    }
    
    void Content::setName(const string &name)
    {
        contentName = name;
    }
    
    
    string Content::getName()
    {
        if (contentName.empty())
        {
            const type_info& id = typeid(*this);
            int stat;
            char *name = abi::__cxa_demangle(id.name(), 0, 0, &stat);
            if (name != NULL && stat == 0) {
                return string(name);
            }
            ofLogWarning(MODULE_NAME) << "faild get object name";
            return "";
        }
        else return contentName;
    }
    
    bool Content::isDebugEnabled() {
        return isDebug;
    }
    
    
    
    //---------------------------------------------------------------------------------------
    /*
     MANAGER CLASS
     */
    //---------------------------------------------------------------------------------------
    
    bool Manager::isValid(const int nid)
    {
        if (mContents.empty() || nid < 0 || nid >= mContents.size())
        {
            ofLogError(MODULE_NAME) << "Manager has not content's ID: " << nid;
            return false;
        }
        return true;
    }
    
    Manager::Manager()
    : bBackgroundUpdate(false)
    , mCurrentContent(0)
    {
        isAudioInEnabled = false;
    }
    
    Manager::~Manager()
    {
    };
    
    void Manager::setup(const float width, const float height, const int internalformat, const int numSamples)
    {
        mFboSettings = ofFbo::Settings();
        mFboSettings.width = width;
        mFboSettings.height = height;
        mFboSettings.internalformat = internalformat;
        mFboSettings.numSamples = numSamples;
        //mFboSettings.depthStencilAsTexture = true;
        mFboSettings.useDepth = true;
    }
    
    void Manager::setupAudio() {
        
        isAudioInEnabled = true;
    }
    
    void Manager::setup(const ofFbo::Settings& settings)
    {
        mFboSettings = settings;
    }
    
    void Manager::update()
    {
        if(isAudioInEnabled) {
            //level = getAudioLevel(10000);
        }
        
        
        for (const auto& e : mContents)
        {
            if (e->opacity > 0.0 || bBackgroundUpdate)
            {
                if(isAudioInEnabled) {
                    //e->obj->setEasyFFT(fft.getBins());
                }
                e->obj->update();
                
                e->fbo.begin();
                ofClear(0,0);
                glPushAttrib(GL_ALL_ATTRIB_BITS);
                ofPushMatrix();
                ofPushStyle();
                e->obj->draw();
                ofPopStyle();
                ofPopMatrix();
                glPopAttrib();
                e->fbo.end();
            }
        }
    }
    
    void Manager::draw(const float x, const float y, const float z, const float width, const float height)
    {
        ofColor currentColor = ofGetStyle().color;
        for (const auto& e : mContents)
        {
            if (e->opacity > 0.0)
            {
                ofPushStyle();
                ofSetColor(currentColor, e->opacity * currentColor.a);
#if (OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR < 9)
                e->fbo.getTextureReference().draw(x, y, z, width, height);
#else
                e->fbo.getTexture().draw(x, y, z, width, height);
#endif
                ofPopStyle();
                
                if(isDebug && e->obj->isGUIEnabled) {
                    e->obj->gui.draw();
                }
            }
        }
    }
    
    void Manager::draw()
    {
        draw(0, 0, 0, mFboSettings.width, mFboSettings.height);
    }
    
    void Manager::draw(const float x, const float y)
    {
        draw(x, y, 0, mFboSettings.width, mFboSettings.height);
    }
    
    void Manager::draw(const float x, const float y, const float width, const float height)
    {
        draw(x, y, 0, width, height);
    }
    
    void Manager::draw(ofRectangle& rectangle)
    {
        draw(rectangle.x, rectangle.y, 0, rectangle.width, rectangle.height);
    }
    
    void Manager::exit()
    {
        for (const auto& e : mContents)
        {
            e->obj->exit();
        }
    }
    
    void Manager::windowResized(int w, int h) {
        
        bufferResized(w, h);
        allocateBuffer(w, h);
        
        for (const auto& e : mContents)
        {
            e->obj->windowResized(w, h);
        }
    }
    
    void Manager::setDebug(bool _val) {
        isDebug = _val;
        
        for (const auto& e : mContents)
        {
            e->obj->isDebug = isDebug;
        }
    }
    
    
    bool Manager::getDebug() {
        return isDebug;
    }
    
    
    void Manager::setOpacity(const int nid, const float opacity)
    {
        if (!isValid(nid)) return;
        mContents[nid]->opacity = ofClamp(opacity, 0.0, 1.0);
    }
    
    void Manager::setOpacity(const string &name, const float opacity)
    {
        for (auto& o : mContents)
        {
            if (o->obj->getName() == name)
            {
                o->opacity = ofClamp(opacity, 0.0, 1.0);
            }
        }
    }
    
    void Manager::setOpacityAll(const float opacity)
    {
        for (auto& e : mContents)
        {
            e->opacity = ofClamp(opacity, 0.0, 1.0);
        }
    }
    
    void Manager::switchContent(const int nid)
    {
        if (!isValid(nid)) return;
        for (int i = 0; i < mContents.size(); ++i)
        {
            if (i == nid)
            {
                mContents[i]->opacity = 1.0;
            }
            else {
                mContents[i]->opacity = 0.0;
            }
        }
        mCurrentContent = nid;
    }
    
    void Manager::switchContent(const string& name)
    {
        for (auto& o : mContents)
        {
            if (o->obj->getName() == name)
            {
                o->opacity = 1.0;
            }
            else {
                o->opacity = 0.0;
            }
        }
    }
    
    void Manager::switchNextContent(bool loop)
    {
        mCurrentContent++;
        if (mCurrentContent >= mContents.size())
        {
            loop ? mCurrentContent = 0 : mCurrentContent = mContents.size() - 1;
        }
        switchContent(mCurrentContent);
    }
    
    void Manager::switchPreviousContent(bool loop)
    {
        mCurrentContent--;
        if (mCurrentContent < 0)
        {
            loop ? mCurrentContent = mContents.size() - 1 : mCurrentContent = 0;
        }
        switchContent(mCurrentContent);
    }
    
    void Manager::enableBackgroundUpdate(bool enable)
    {
        bBackgroundUpdate = enable;
    }
    
    void Manager::allocateBuffer(const float width, const float height, const int internalformat, const int numSamples)
    {
        setup(width, height, internalformat, numSamples);
        allocateBuffer(mFboSettings);
    }
    
    void Manager::allocateBuffer(const ofFbo::Settings& settings)
    {
        for (auto& o : mContents)
        {
            o->fbo.allocate(settings);
            o->obj->bufferWidth  = settings.width;
            o->obj->bufferHeight = settings.height;
            o->obj->bufferResized(settings.width, settings.height);
        }
    }
    
    
    void Manager::addContent(ofxContentsManager::Content *newContentPtr)
    {
        setupContent(newContentPtr);
    }
    
    
    bool Manager::removeContent(const int nid)
    {
        if (!isValid(nid)) return false;
        contents_it it = mContents.begin() + nid;
        myContent* o = *it;
        o->obj->exit();
        o->opacity.removeListener(o->obj, &Content::onOpacityChanged);
        delete o->obj;
        delete o;
        mContents.erase(it);
        return true;
    }
    
    void Manager::removeContent(const string& name)
    {
        contents_it it = mContents.begin();
        while (it != mContents.end())
        {
            if ((*it)->obj->getName() == name)
            {
                myContent* o = *it;
                o->obj->exit();
                o->opacity.removeListener(o->obj, &Content::onOpacityChanged);
                delete o->obj;
                delete o;
                it = mContents.erase(it);
            }
            else ++it;
        }
    }
    
    Content* Manager::getContent(const int nid)
    {
        if (!isValid(nid)) return NULL;
        contents_it it = mContents.begin() + nid;
        return (*it)->obj;
    }
    
    vector<Content*> Manager::getAllContents()
    {
        vector<Content*> dst;
        contents_it it = mContents.begin();
        while (it != mContents.end())
        {
            dst.push_back((*it)->obj);
            ++it;
        }
        return dst;
    }
    
    int Manager::getNumContents()
    {
        return mContents.size();
    }
    
    const ofParameterGroup& Manager::getOpacityParameterGroup(const string& groupName)
    {
        mOpacityParams.setName(groupName);
        return mOpacityParams;
    }
    
    void Manager::clear()
    {
        typename vector<myContent*>::iterator it = mContents.begin();
        while (it != mContents.end())
        {
            myContent* o = *it;
            o->obj->exit();
            delete o->obj;
            delete o;
        }
        mContents.clear();
        mOpacityParams.clear();
    }
}
