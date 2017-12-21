/* Copyright 2014-2017 Rsyn
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
/*
 * Session.h
 *
 *  Created on: May 8, 2015
 *      Author: jucemar
 */

#ifndef RSYN_SESSION_H
#define RSYN_SESSION_H

#include <string>

#include "rsyn/session/Service.h"
#include "rsyn/session/Process.h"
#include "rsyn/session/Reader.h"
#include "rsyn/session/Message.h"

#include "rsyn/3rdparty/json/json.hpp"

// To be spun off as a 3rd party...
#include "rsyn/io/parser/script/ScriptCommand.h"

#include "rsyn/core/Rsyn.h"
#include "rsyn/util/Logger.h"
#include "rsyn/util/Units.h"

namespace Rsyn {

typedef nlohmann::json Json;

enum GraphicsEvent {
	GRAPHICS_EVENT_DESIGN_LOADED,
	GRAPHICS_EVENT_REFRESH,
	GRAPHICS_EVENT_UPDATE_OVERLAY_LIST
}; // end enum

typedef std::function<Service *()> ServiceInstantiatonFunction;
typedef std::function<Process *()> ProcessInstantiatonFunction;
typedef std::function<Reader *()> ReaderInstantiatonFunction;

////////////////////////////////////////////////////////////////////////////////
// Session Data
////////////////////////////////////////////////////////////////////////////////

struct SessionData {
	Message msgMessageRegistrationFail;

	////////////////////////////////////////////////////////////////////////////
	// Session Variables
	////////////////////////////////////////////////////////////////////////////
	
	std::map<std::string, Json> clsSessionVariables;

	////////////////////////////////////////////////////////////////////////////
	// Message System
	////////////////////////////////////////////////////////////////////////////

	MessageManager clsMessageManager;

	////////////////////////////////////////////////////////////////////////////
	// Services
	////////////////////////////////////////////////////////////////////////////
	
	// Generic functions used to instantiate optimizers by name.
	std::unordered_map<std::string, ServiceInstantiatonFunction> clsServiceInstanciationFunctions;
	std::unordered_map<std::string, Service *> clsRunningServices;
	
	////////////////////////////////////////////////////////////////////////////
	// Processes
	////////////////////////////////////////////////////////////////////////////
	
	// Generic functions used to instantiate optimizers by name.
	std::unordered_map<std::string, ProcessInstantiatonFunction> clsProcesses;
	
	////////////////////////////////////////////////////////////////////////////
	// Loader
	////////////////////////////////////////////////////////////////////////////
	
	// Generic functions used to instantiate optimizers by name.
	std::unordered_map<std::string, ReaderInstantiatonFunction> clsReaders;

	////////////////////////////////////////////////////////////////////////////
	// Design
	////////////////////////////////////////////////////////////////////////////

	Rsyn::Design clsDesign;
	Rsyn::Module clsModule;
		
	////////////////////////////////////////////////////////////////////////////
	// Logger
	////////////////////////////////////////////////////////////////////////////
	Logger* logger;
	
	////////////////////////////////////////////////////////////////////////////
	// Configuration
	////////////////////////////////////////////////////////////////////////////
	std::string clsInstallationPath;
	bool clsVerbose = false;
	
	////////////////////////////////////////////////////////////////////////////
	// Script
	////////////////////////////////////////////////////////////////////////////
	ScriptParsing::CommandManager clsCommandManager;
	
	std::function<void(const GraphicsEvent event)> clsGraphicsCallback = nullptr;
}; // end struct

////////////////////////////////////////////////////////////////////////////////
// Session Proxy
////////////////////////////////////////////////////////////////////////////////

class Session : public Rsyn::Proxy<SessionData> {
public:
	Session() {
		data = sessionData;
	};
	
	Session(std::nullptr_t) { 
		data = nullptr;
	}
	
	Session &operator=(const Session &other) {
		data = other.data;
		return *this;
	}	
	
	static void init();

	//! @note If I understood the C++ standard correctly, all static variables
	//!       are first zero-initialized and then all constructors, if any, are
	//!       called. In that case, it is safe to check if the session was
	//!       already initialized by checking if the session data pointer is
	//!       null as it would be zero-initialized before any more sophisticated
	//!       initialization (e.g. constructor) is called. Hopefully this is the
	//!       case and we don't have any risk of falling into the "static
	//!       variable order initialization fiasco".
	static bool isInitialized() {
		return sessionData;
	} // end method

private:

	static SessionData * sessionData;

	////////////////////////////////////////////////////////////////////////////
	// Session Variables
	////////////////////////////////////////////////////////////////////////////

public:

	static void setSessionVariable(const std::string &name, const Json &value) {
		sessionData->clsSessionVariables[name] = value;
	} // end method

	static void unsetSessionVariable(const std::string &name) {
		sessionData->clsSessionVariables.erase(name);
	} // end method

	static const bool getSessionVariableAsBool(const std::string &name, const bool defaultValue = false) {
		auto it = sessionData->clsSessionVariables.find(name);
		return (it != sessionData->clsSessionVariables.end())? it->second.get<bool>() : defaultValue;
	} // end method

	static const int getSessionVariableAsInteger(const std::string &name, const int defaultValue = 0) {
		auto it = sessionData->clsSessionVariables.find(name);
		return (it != sessionData->clsSessionVariables.end())? it->second.get<int>() : defaultValue;
	} // end method

	static const float getSessionVariableAsFloat(const std::string &name, const float defaultValue = 0.0f) {
		auto it = sessionData->clsSessionVariables.find(name);
		return (it != sessionData->clsSessionVariables.end())? it->second.get<float>() : defaultValue;
	} // end method

	static const std::string getSessionVariableAsString(const std::string &name, const std::string &defaultValue = "") {
		auto it = sessionData->clsSessionVariables.find(name);
		return (it != sessionData->clsSessionVariables.end())? it->second.get<std::string>() : defaultValue;
	} // end method

	static const Json getSessionVariableAsJson(const std::string &name, const Json &defaultValue = {}) {
		auto it = sessionData->clsSessionVariables.find(name);
		return (it != sessionData->clsSessionVariables.end())? it->second : defaultValue;
	} // end method
	
	////////////////////////////////////////////////////////////////////////////
	// Services
	////////////////////////////////////////////////////////////////////////////

	// Register services.
	static void registerServices();

public:
	
	// Helper class to allow seamless casting from a Service pointer to any type
	// which implements operator=(Service *) is is directly compatible.
	
	class ServiceHandler {
	private:
		Service *clsService;
		
	public:
		ServiceHandler(Service *service) : clsService(service) {}
		
		template<typename T>
		operator T *() {
			T * pointer = dynamic_cast<T *>(clsService);
			if (pointer != clsService) {
				std::cout << "ERROR: Trying to cast a service to the wrong type.\n";
				throw Exception("Trying to cast a service to the wrong type.");
			} // end if
			return pointer;
		} // end operator
	}; // end class
		
	// Register a service.
	template<typename T>
	static void registerService(const std::string &name) {
		auto it = sessionData->clsServiceInstanciationFunctions.find(name);
		if (it != sessionData->clsServiceInstanciationFunctions.end()) {
			std::cout << "ERROR: Service '" << name << "' was already "
					"registered.\n";
			std::exit(1);
		} else {
			if (sessionData->clsVerbose) {
				std::cout << "Registering service '" << name << "' ...\n";
			} // end if
			sessionData->clsServiceInstanciationFunctions[name] = []() -> Service *{
				return new T();
			};
		} // end else
	} // end method
	
	// Start a service.
	static bool startService(const std::string &name, const Json &params = {}) {
		auto it = sessionData->clsServiceInstanciationFunctions.find(name);
		if (it == sessionData->clsServiceInstanciationFunctions.end()) {
			std::cout << "ERROR: Service '" << name << "' was not "
					"registered.\n";
			std::exit(1);
			return false;
		} else {
			Service * service = getServiceInternal(name);
			if (!service) {
				service = it->second();
				service->start(params);
				sessionData->clsRunningServices[name] = service;
				postGraphicsEvent(GRAPHICS_EVENT_UPDATE_OVERLAY_LIST);
				return true;
			} else {
				std::cout << "WARNING: Service '" << name << "' is already running.\n";
				return false;
			} // end else
		} // end else
	} // end method
	
	// Gets a running service.
	static ServiceHandler getService(const std::string &name,
			const ServiceRequestType requestType = SERVICE_MANDATORY) {
		Service *service = getServiceInternal(name);
		if (!service && (requestType == SERVICE_MANDATORY)) {
			std::cout << "ERROR: Service '" << name << "' was not started.\n";
			throw Exception("ERROR: Service '" + name + "' was not started");
		} // end if
		return ServiceHandler(service);
	} // end method
	
	// Checks if a service is running.
	static bool isServiceRunning(const std::string &name) {
		return getServiceInternal(name) != nullptr;
	} // end method
	
private:
	
	static Service * getServiceInternal(const std::string &name) {
		auto it = sessionData->clsRunningServices.find(name);
		return it == sessionData->clsRunningServices.end()? nullptr : it->second;
	} // end method

	static void listService(std::ostream & out = std::cout) {
		out<<"List of services ";
		out<<"([R] -> Running, [S] -> Stopped):\n";
		// print only running services
		for (std::pair<std::string, ServiceInstantiatonFunction> srv : sessionData->clsServiceInstanciationFunctions) {
			if (!isServiceRunning(srv.first))
				continue;
			out << "\t[R] " << srv.first << "\n";
		} // end for 
		// print only stopped services 
		for (std::pair<std::string, ServiceInstantiatonFunction> srv : sessionData->clsServiceInstanciationFunctions) {
			if (isServiceRunning(srv.first))
				continue;
			out << "\t[S] "<<srv.first << "\n";
		} // end for 
		out << "--------------------------------------\n";
	} /// end method 
	
	////////////////////////////////////////////////////////////////////////////
	// Processes
	////////////////////////////////////////////////////////////////////////////
private:
	static void listProcess(std::ostream & out = std::cout) {
		out<<"List of registered processes:\n";
		for(std::pair<std::string, ProcessInstantiatonFunction> process : sessionData->clsProcesses) {
			out<<"\t"<<process.first<<"\n";
		} // end for 
		out<<"--------------------------------------\n";
	} // end method

	// Register processes.
	static void registerProcesses();

public:	
	
	// Register a process.
	template<typename T>
	static void registerProcess(const std::string &name) {
		auto it = sessionData->clsProcesses.find(name);
		if (it != sessionData->clsProcesses.end()) {
			std::cout << "ERROR: Process '" << name << "' was already "
					"registered.\n";
			std::exit(1);
		} else {
			if (sessionData->clsVerbose) {
				std::cout << "Registering process '" << name << "' ...\n";
			} // end if
			sessionData->clsProcesses[name] = []() -> Process *{
				return new T();
			};
		} // end else
	} // end method
	
	// Run an optimizer.
	static bool runProcess(const std::string &name, const Json &params = {}) {
		bool result = false;
		
		auto it = sessionData->clsProcesses.find(name);
		if (it == sessionData->clsProcesses.end()) {
			std::cout << "ERROR: Process '" << name << "' was not "
					"registered.\n";
		} else {
			std::unique_ptr<Process> opto(it->second());
			result = opto->run(params);
		} // end else
		
		return result;
	} // end method
		
	////////////////////////////////////////////////////////////////////////////
	// Reader
	////////////////////////////////////////////////////////////////////////////
private:
	static void listReader(std::ostream & out = std::cout) {
		out<<"List of registered Readers:\n";
		for(std::pair<std::string, ReaderInstantiatonFunction> reader : sessionData->clsReaders) {
			out<<"\t"<<reader.first<<"\n";
		} // end for 
		out<<"--------------------------------------\n";
	} /// end method

	// Register loader.
	static void registerReaders();

public:
		
	// Register a loader.
	template<typename T>
	static void registerReader(const std::string &name) {
		auto it = sessionData->clsReaders.find(name);
		if (it != sessionData->clsReaders.end()) {
			std::cout << "ERROR: Reader '" << name << "' was already "
					"registered.\n";
			std::exit(1);
		} else {
			if (sessionData->clsVerbose) {
				std::cout << "Registering reader '" << name << "' ...\n";
			} // end if
			sessionData->clsReaders[name] = []() -> Reader *{
				return new T();
			};
		} // end else
	} // end method
	
	// Run an loader.
	static void runReader(const std::string &name, const Json &params = {}) {
		auto it = sessionData->clsReaders.find(name);
		if (it == sessionData->clsReaders.end()) {
			std::cout << "ERROR: Reader '" << name << "' was not "
					"registered.\n";
		} else {
			std::unique_ptr<Reader> opto(it->second());
			opto->load(params);
			postGraphicsEvent(GRAPHICS_EVENT_DESIGN_LOADED);
		} // end else
	} // end method

	////////////////////////////////////////////////////////////////////////////
	// Messages
	////////////////////////////////////////////////////////////////////////////

private:
	static void registerMessages();

public:

	static void registerMessage(const std::string &label, const MessageLevel &level, const std::string &title, const std::string &msg = "");
	static Message getMessage(const std::string &label);
	
	////////////////////////////////////////////////////////////////////////////
	// Misc
	////////////////////////////////////////////////////////////////////////////

	static Rsyn::Design getDesign() { return sessionData->clsDesign; }

	static const std::string &getInstallationPath() { return sessionData->clsInstallationPath; }

	////////////////////////////////////////////////////////////////////////////
	// Script
	////////////////////////////////////////////////////////////////////////////
private:

	static void registerDefaultCommands();
	
public:	

	typedef std::function<void(const ScriptParsing::Command &command)> CommandHandler;
	
	static void registerCommand(const ScriptParsing::CommandDescriptor &dscp, const CommandHandler handler);
	
	static void evaluateString(const std::string &str);
	static void evaluateFile(const std::string &filename);

	static ScriptParsing::CommandManager &getCommandManager() {
		return sessionData->clsCommandManager;
	} // end method

	////////////////////////////////////////////////////////////////////////////
	// Graphics
	////////////////////////////////////////////////////////////////////////////
public:

	static void registerGraphicsCallback(const std::function<void(const GraphicsEvent event)> &callback) {
		sessionData->clsGraphicsCallback = callback;
	} // end method

	static void postGraphicsEvent(const GraphicsEvent event) {
		if (sessionData->clsGraphicsCallback)
			sessionData->clsGraphicsCallback(event);
	} // end method

}; // end class

////////////////////////////////////////////////////////////////////////////////
// Startup
////////////////////////////////////////////////////////////////////////////////

// Helper class used to perform component initialization during the application
// startup. Declare a startup object in a cpp file:
//
// Rsyn::Startup startup([]{
//     Rsyn::Session::registerService(...);
//     Rsyn::Session::registerProcess(...);
//     Rsyn::Session::registerMessage(...);
// }); // end startup
//
// This will construct a global object that will be called during the
// application initialization.

class Startup {
public:
	Startup(std::function<void()> f) {
		if (!Session::isInitialized())
			Session::init();
		f();
	} // end constructor
}; // end class

} // end namespace

#endif /* INFRA_ICCAD15_SESSION_H_ */
