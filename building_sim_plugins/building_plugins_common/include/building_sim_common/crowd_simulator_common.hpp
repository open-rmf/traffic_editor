#ifndef BUILDING_SIM_COMMON__CROWD_SIMULATOR_COMMON_HPP
#define BUILDING_SIM_COMMON__CROWD_SIMULATOR_COMMON_HPP

#include <functional>
#include <list>
#include <queue>
#include <memory>
#include <regex> //for parsing initial_pose

#include <MengeCore/Runtime/SimulatorDB.h>
#include <MengeCore/Orca/ORCADBEntry.h>
#include <MengeCore/Orca/ORCASimulator.h>
#include <MengeCore/PluginEngine/CorePluginEngine.h>

#include <rclcpp/rclcpp.hpp>

namespace crowd_simulator {

using AgentPtr = std::shared_ptr<Menge::Agents::BaseAgent>;
//===============================================================
/*
* class AgentPose3d
* replace the common interface of ignition::math::Pose3d, which might apply with different version in different plugins
*/
class  AgentPose3d{

public:
  AgentPose3d() : _x(0), _y(0), _z(0), _pitch(0), _roll(0), _yaw(0) {}
  AgentPose3d(double& x, double& y, double& z, double& pitch, double& roll, double& yaw)
    : _x(x), _y(y), _z(z), _pitch(pitch), _roll(roll), _yaw(yaw) {}

  ~AgentPose3d() {}

  double X() const;
  double Y() const;
  double Z() const;
  double Pitch() const;
  double Roll() const;
  double Yaw() const;

  double& X();
  double& Y();
  double& Z();
  double& Pitch();
  double& Roll();
  double& Yaw();

  void X(const double& x);
  void Y(const double& y);
  void Z(const double& z);
  void Pitch(const double& pitch);
  void Roll(const double& roll);
  void Yaw(const double& yaw);

private:
  double _x;
  double _y;
  double _z;
  double _pitch;
  double _roll;
  double _yaw;

};

//================================================================
/*
* class MengeHandle
*/
class MengeHandle
{
public:

  MengeHandle(const std::string& resourcePath,
    const std::string& behaviorFile,
    const std::string& sceneFile,
    const float simTimeStep = 0.0
  )
  : _resourcePath(resourcePath),
    _behaviorFile(behaviorFile),
    _sceneFile(sceneFile),
    _simTimeStep(simTimeStep),
    _agentCount(0)
  {

    _behaviorFile = this->_ResourceFilePath(_behaviorFile);
    _sceneFile = this->_ResourceFilePath(_sceneFile);

    if (this->_LoadSimulation())
    {
      this->initialized = true;
    }
    assert(this->initialized);
  }

  bool initialized = false;

  void SetSimTimeStep(float simTimeStep);
  float GetSimTimeStep();
  size_t GetAgentCount();
  void SimStep(); //proceed one-time simulation step in _sim

  AgentPtr GetAgent(size_t id);

private:
  std::string _resourcePath;
  std::string _behaviorFile;
  std::string _sceneFile;
  float _simTimeStep;
  size_t _agentCount;
  
  //Have some problem when transfer raw pointer to shared_ptr or unique_ptr
  std::shared_ptr<Menge::Agents::SimulatorInterface> _sim;

  std::string _ResourceFilePath(const std::string& relativePath) const;
  bool _LoadSimulation(); //initialize simulatorinterface

};

//================================================================
/*
* class ModelTypeDatabase
*/
class ModelTypeDatabase
{
public:
  struct Record
  {
    std::string typeName;
    std::string fileName;
    AgentPose3d pose;
    std::string animation;
    double animationSpeed;
    
    //for ignition
    std::string modelFilePath;
  };

  using RecordPtr = std::shared_ptr<Record>;

  ModelTypeDatabase() {}

  //Create a new record and returns a reference to the record
  template<typename... Args>
  RecordPtr Emplace(Args&& ... args){
    auto pair = this->_records.emplace(std::forward<Args>(args)...); //return pair<iterator, bool>
    assert(pair.second);
    return pair.first->second;
  }

  //Get the total number of actors
  size_t Size();

  RecordPtr Get(const std::string& typeName);

private:
  std::unordered_map<std::string, RecordPtr> _records;
};

//================================================================
/*
* class CrowdSimInterface
* provides the relationship between menge agents and gazebo models
* provides the interface to set position between gazebo models and menge agents
*/
class CrowdSimInterface
{

public:
  struct Object
  {
    AgentPtr agentPtr;
    std::string modelName;
    std::string typeName;
    bool isExternal = false;
  };

  using ObjectPtr = std::shared_ptr<Object>;

  CrowdSimInterface(const std::string& resourcePath,
    const std::string& behaviorFile,
    const std::string& sceneFile,
    float simTimeStep = 0.0
  )
  {
    this->_mengeHandle = std::make_shared<MengeHandle>(resourcePath,
        behaviorFile, sceneFile,
        simTimeStep);
  }

  rclcpp::Logger logger() const;

  template<typename SdfPtrT>
  bool ReadSDF(SdfPtrT& sdf);

  bool SpawnObject(std::vector<std::string>& externalModels);
  void AddObject(AgentPtr agentPtr, const std::string& modelName,
    const std::string& typeName, bool isExternal);

  size_t GetNumObjects();
  ObjectPtr GetObjectById(size_t id);

  void OneStepSim();

  void UpdateExternalAgent(size_t id, const AgentPose3d& modelPose);
  void UpdateExternalAgent(const AgentPtr agentPtr, const AgentPose3d& modelPose);
  void GetAgentPose(size_t id, double deltaSimTime, AgentPose3d& modelPose);
  void GetAgentPose(const AgentPtr agentPtr, double deltaSimTime, AgentPose3d& modelPose);

private:
  std::vector<ObjectPtr> _objects; //Database, use id to access ObjectPtr
  std::shared_ptr<MengeHandle> _mengeHandle;
  std::shared_ptr<ModelTypeDatabase> _modelTypeDBPtr;

  float _simTimeStep;
  std::string _resourcePath;
  std::string _behaviorFile;
  std::string _sceneFile;
  std::vector<std::string> _externalAgents;

  template<typename SdfPtrT>
  bool _LoadModelInitPose(SdfPtrT& modelTypeElement, AgentPose3d& result) const;

};

template<typename SdfPtrT>
bool CrowdSimInterface::ReadSDF(SdfPtrT& sdf) 
{
  if (!sdf->template HasElement("resource_path"))
  {
    char* menge_resource_path;
    menge_resource_path = getenv("MENGE_RESOURCE_PATH");
    RCLCPP_ERROR(logger(), 
      "No resource path provided! <env MENGE_RESOURCE_PATH> " + std::string(menge_resource_path) + " will be used." ); 
    this->_resourcePath = std::string(menge_resource_path);
  } else{
    this->_resourcePath = sdf->template GetElement("resource_path")->template Get<std::string>();
  }
  
  if (!sdf->template HasElement("behavior_file"))
  {
    RCLCPP_ERROR(logger(), 
      "No behavior file found! <behavior_file> Required!" ); 
    return false;
  }
  this->_behaviorFile = sdf->template GetElement("behavior_file")->template Get<std::string>();

  if (!sdf->template HasElement("scene_file"))
  {
    RCLCPP_ERROR(logger(), 
      "No scene file found! <scene_file> Required!" );
    return false;
  }
  this->_sceneFile = sdf->template GetElement("scene_file")->template Get<std::string>();

  if (!sdf->template HasElement("update_time_step"))
  {
    RCLCPP_ERROR(logger(), 
      "No update_time_step found! <update_time_step> Required!");
    return false;
  }
  this->_simTimeStep = sdf->template GetElement("update_time_step")->template Get<float>();

  if (!sdf->template HasElement("model_type"))
  {
    RCLCPP_ERROR(logger(), 
      "No model type for agents found! <model_type> element Required!");
    return false;
  }
  auto modelTypeElement = sdf->template GetElement("model_type");
  while (modelTypeElement)
  {
    std::string s;
    if (!modelTypeElement->template Get<std::string>("typename", s, ""))
    {
      RCLCPP_ERROR(logger(), 
        "No model type name configured in <model_type>! <typename> Required");
      return false;
    }

    auto modelTypePtr = this->_modelTypeDBPtr->Emplace(s, new crowd_simulator::ModelTypeDatabase::Record()); //unordered_map
    modelTypePtr->typeName = s;

    if (!modelTypeElement->template Get<std::string>("filename", modelTypePtr->fileName,""))
    {
      RCLCPP_ERROR(logger(), 
        "No actor skin configured in <model_type>! <filename> Required");
      return false;
    }

    if (!modelTypeElement->template Get<std::string>("animation", modelTypePtr->animation, ""))
    {
      RCLCPP_ERROR(logger(), 
        "No animation configured in <model_type>! <animation> Required");
      return false;
    }

    if (!modelTypeElement->template Get<double>("animation_speed", modelTypePtr->animationSpeed, 0.0))
    {
      RCLCPP_ERROR(logger(), 
        "No animation speed configured in <model_type>! <animation_speed> Required");
      return false;
    }

    if (!modelTypeElement->template HasElement("initial_pose"))
    {
      RCLCPP_ERROR(logger(), 
        "No model initial pose configured in <model_type>! <initial_pose> Required [" + s + "]");
      return false;
    }
    if (!this->_LoadModelInitPose(modelTypeElement, modelTypePtr->pose))
    {
      RCLCPP_ERROR(logger(), 
        "Error loading model initial pose in <model_type>! Check <initial_pose> in [" + s + "]");
      return false;
    }

    modelTypeElement = modelTypeElement->template GetNextElement("model_type");
  }

  if (!sdf->template HasElement("external_agent"))
  {
    RCLCPP_ERROR(logger(), 
      "No external agent provided. <external_agent> is needed with a unique name defined above.");
  }
  auto externalAgentElement = sdf->template GetElement("external_agent");
  while (externalAgentElement)
  {
    auto exAgentName = externalAgentElement->template Get<std::string>();
    RCLCPP_ERROR(logger(), 
      "Added external agent: [ " + exAgentName + " ].");
    this->_externalAgents.emplace_back(exAgentName); //just store the name
    externalAgentElement = externalAgentElement->template GetNextElement("external_agent");
  }

  return true;
}

template<typename SdfPtrT>
bool CrowdSimInterface::_LoadModelInitPose(SdfPtrT& modelTypeElement, AgentPose3d& result) const
{
  std::string poseStr;
  if (modelTypeElement->template Get<std::string>("initial_pose", poseStr, ""))
  {
    std::regex ws_re("\\s+"); //whitespace
    std::vector<std::string> parts(
      std::sregex_token_iterator(poseStr.begin(), poseStr.end(), ws_re, -1),
      std::sregex_token_iterator());

    if (parts.size() != 6)
    {
      RCLCPP_ERROR(logger(), 
        "Error loading <initial_pose> in <model_type>, 6 floats (x, y, z, pitch, roll, yaw) expected.");
      return false;
    }

    result.X( std::stod(parts[0]) );
    result.Y( std::stod(parts[1]) );
    result.Z( std::stod(parts[2]) );
    result.Pitch( std::stod(parts[3]) );
    result.Roll( std::stod(parts[4]) );
    result.Yaw( std::stod(parts[5]) );
  }
  return true;
}

} //namespace crowd_simulator


#endif //CROWD_SIMULATION_COMMON__CROWD_SIMULATOR_COMMON_HPP