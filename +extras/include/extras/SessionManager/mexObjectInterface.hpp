#pragma once

#include <extras/cmex/mexextras.hpp>
#include "ObjectManager.h"
#include <extras/string_extras.hpp>
#include <functional>
#include <unordered_map>
#include <extras/cmex/MxStruct.hpp>
#include <cstring>
#include <extras/string_extras.hpp>

namespace extras {namespace SessionManager {


	namespace Member {
		/// @brief Generic Interface class for all member types (e.g. static/non-static function/variable)
		class IMemberT {
		public:
			enum class MemberStatic {
				member_nonstatic = 0,
				member_static = 1
			};
			enum class MemberAccess {
				member_public = 0,
				member_protected,
				member_private
			};
			enum class MemberVisibility {
				member_hidden = 0,
				member_nothidden = 1
			};
			enum class MemberType {
				member_function = 0,
				member_variable
			};
		protected:
			MemberStatic memberStatic;
			MemberType memberType;
			MemberVisibility memberVisibility;
			MemberAccess memberAccess;
		public:
			virtual ~IMemberT() {};
			IMemberT(MemberType t, MemberStatic s, MemberAccess a, MemberVisibility v) :
				memberStatic(s),
				memberType(t),
				memberAccess(a),
				memberVisibility(v)
			{}

			MemberAccess access() const { return memberAccess; }
			MemberVisibility visibility() const { return memberVisibility; }
			MemberStatic memStatic() const { return memberStatic; }
			bool isStatic() const { return memberStatic == MemberStatic::member_static; }
			MemberType type() const { return memberType; }
			bool isFunction() const { return memberType == MemberType::member_function; }
			bool isVariable() const { return memberType == MemberType::member_variable; }
			bool isHidden() const { return memberVisibility == MemberVisibility::member_hidden; }
		};

		/// @brief Interface class for all function type members
		class IMemberFunctionT : public IMemberT {
		public:
			virtual ~IMemberFunctionT() {};
			IMemberFunctionT(IMemberT::MemberStatic s, IMemberT::MemberAccess a, IMemberT::MemberVisibility v):
				IMemberT(IMemberT::MemberType::member_function, s, a, v)
			{};

			virtual void execute(int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs) const = 0;
			virtual void operator()(int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs) const = 0;
		};

		/// @brief Interface class for member variables
		class IMemberVariableT : public IMemberT {
		public:
			virtual ~IMemberVariableT() {};
			IMemberVariableT(IMemberT::MemberStatic s, IMemberT::MemberAccess a, IMemberT::MemberVisibility v) :IMemberT(IMemberT::MemberType::member_variable, s, a, v) {};

			virtual bool isSettable() const = 0;
			virtual bool writable() const { return isSettable(); }

			virtual void get(int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs) const = 0;
			virtual void set(int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs) const = 0;

		};
	}

	/**
	* @brief mexFuncton Class manager wrapper.
	*
	* Use this to wrap c++ objects so that their methods can be called from MATLAB
	*
	* Usage:
	* Your mex cpp file should look like this
	*
	*     `mexInterfaceManager<YOUR_CLASS_TYPE> MexInt; //global instance of mexInterface, persists between mexFunction calls
	*
	*     void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){
	*         MexInt.mexFunction(nlhs,plhs,nrhs,prhs);
	*     }`
	*
	* In MATLAB you would call
	*     >> p_obj = YOUR_MEX_FUNCTION('new'); %get pointer to c++ object
	*     >> YOUR_MEX_FUNCTION('your_method',p_obj,Arg1,Arg2,...); %run method, note: must have extended mexInterface to implement your_method
	*     >> YOUR_MEX_FUNCTION('delete',p_obj); %delete associated object
	*
	* You can get A list of the methods by calling
	*	  >> metNames = YOU_MEX_FUNCTION('getMethodNames');
	* which will return a cellstr array specifying all of the method names
	*
	* Extending mexInterface:
	*  You should extend mexInterface for your object as follows
	*
	*    `class yourMexInterface: public mexInterfaceManager<yourClass>{
	*         /// implement function named 'your_method'
	*         void your_method(int nlhs, mxArray* plhs[],int nrhs, const mxArray* prhs[]){
	*            if (nrhs < 1) {
	*                 throw(std::runtime_error("requires intptr argument specifying object to destruct"));
	*             }
	*             ObjManager.get(prhs[0])->yourMethod(...); //yourClass should have public member function yourMethod(...)
	*         }
	*     public:
	*         yourMexInterface(){
	*             using namespace std::placeholders;
	*             addFunction('your_method',std::bind(&yourMexInterface::your_method,*this,_1,_2,_3,_4)); //add your_method to function list
	*         }
	*     };`
	 * @tparam ObjType the class type you want to manager
	*/
	template<class ObjType>
	class mexObjectInterface : virtual protected ObjectManager<ObjType> {
		typedef std::function<void(int, mxArray**, int, const mxArray**)> MEX_SF; /// function object for mexFunction to static functions
		typedef std::function<void(int, mxArray**)> MEX_SF_noRHS;/// function object for mexFunction to static functions not allowing rhs arguments
		typedef std::function<void(std::shared_ptr<ObjType>, int, mxArray**, int, const mxArray**)> MEX_F; /// function object for mexFunction to nonstatic functions
		typedef std::function<void(std::shared_ptr<ObjType>, int, mxArray**)> MEX_F_noRHS;/// function object for mexFunction to nonstatic functions not allowing rhs arguments

		typedef std::unordered_map<std::string, Member::IMemberT> MemberMapT; /// map type used to store members

		/// @brief Container for Static member function
		class StaticMemberFunction : public virtual Member::IMemberFunctionT {
		protected:
			MEX_SF _function;
		public:
			~StaticMemberFunction() {};
			StaticMemberFunction(MEX_SF func, Member::IMemberT::MemberAccess a, Member::IMemberT::MemberVisibility v) :
				Member::IMemberFunctionT(Member::IMemberT::MemberStatic::member_static, a, v),
				_function(func) {};

			void execute(int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs) const { _function(nlhs, plhs, nrhs, prhs); }
			void operator()(int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs) const { execute(nlhs, plhs, nrhs, prhs); }
		};

		/// @brief Container for non-static member functions
		class NonStaticMemberFunction : public Member::IMemberFunctionT {
		protected:
			MEX_F _function;
			mexObjectInterface<ObjType>* _parent;
		public:
			~NonStaticMemberFunction() {};
			NonStaticMemberFunction(mexObjectInterface<ObjType>* parent, MEX_F func, Member::IMemberT::MemberAccess a, Member::IMemberT::MemberVisibility v) :
				Member::IMemberFunctionT(Member::IMemberT::MemberStatic::member_nonstatic, a, v),
				_parent(parent),
				_function(func) {};
			void execute(int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs) const {
				_function(_parent->getObjectPtr(nrhs, prhs), nlhs, plhs, nrhs - 1, nrhs > 1 ? &(prhs[1]) : nullptr);
			}
			void operator()(int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs) const { execute(nlhs, plhs, nrhs, prhs); }
		};

		

		/// @brief Container for static member variables
		class StaticMemberVariable : public Member::IMemberVariableT {
		protected:
			MEX_SF_noRHS _get;
			MEX_SF _set;
		public:
			~StaticMemberVariable() {};
			StaticMemberVariable(MEX_SF_noRHS get_fn, MEX_SF set_fn, Member::IMemberT::MemberAccess a, Member::IMemberT::MemberVisibility v) :
				IMemberVariableT(Member::IMemberT::MemberStatic::member_static, a, v),
				_get(get_fn),
				_set(set_fn) {};
			bool isSettable() const { return (bool)_set; }
			void get(int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs) const {
				_get(nlhs, plhs);
			}
			void set(int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs) const {
				_set(nlhs, plhs, nrhs, prhs);
			}
		};

		/// @brief container for non-static member variables
		class NonStaticMemberVariable : public Member::IMemberVariableT {
		protected:
			MEX_F_noRHS _get;
			MEX_F _set;
			mexObjectInterface<ObjType>* _parent;
		public:
			~NonStaticMemberVariable() {};
			NonStaticMemberVariable(mexObjectInterface<ObjType>* parent,MEX_F_noRHS get_fn, MEX_F set_fn, Member::IMemberT::MemberAccess a, Member::IMemberT::MemberVisibility v) :
				Member::IMemberVariableT(Member::IMemberT::MemberStatic::member_nonstatic, a, v),
				_parent(parent),
				_get(get_fn),
				_set(set_fn) {};
			bool isSettable() const { return (bool)_set; }
			void get(int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs) const {
				_get(_parent->getObjectPtr(nrhs, prhs), nlhs, plhs);
			}
			void set(int nlhs, mxArray** plhs, int nrhs, const mxArray** prhs) const {
				_set(_parent->getObjectPtr(nrhs, prhs), nlhs, plhs, nrhs - 1, nrhs > 1 ? &(prhs[1]) : nullptr);
			}
		};

	private:
		std::unordered_map<std::string, Member::IMemberT> memberMap; //map of member variables and functions
	protected:
		/// implement 'new' function interface
		virtual void new_object(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
#ifdef DAN_DEBUG
			mexPrintf("mexInterfaceManager<%s>::new_object\n", typeid(ObjType).name());
			mexPrintf("\t press a key to continue\n");
			mexEvalString("pause()");
#endif
			int64_t val = ObjectManager<ObjType>::create(new ObjType); //CHANGE THIS LINE
#ifdef DAN_DEBUG
			mexPrintf("\tObjPtr: %p\n", val);
			mexPrintf("\t press a key to continue\n");
			mexEvalString("pause()");
#endif
			plhs[0] = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
			*((int64_t*)mxGetData(plhs[0])) = val;
		}

		/// implement 'delete' function interface
		void delete_object(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
		{
			if (nrhs < 1) {
				throw(extras::stacktrace_error("requires intptr argument specifying object to destruct"));
			}
			ObjectManager<ObjType>::destroy(prhs[0]);
		}

		/// @brief Add a static member function interface
		/// @param name member function name keyword
		/// @param func std::function<> accepting MEX arguments, used to execute member function
		/// @param a function access flag (e.g. member_protected, member_public,...)
		/// @param v function visibility flag (member_hidden, member_nothidden)
		virtual void addStaticFunction(std::string name, MEX_SF func,
			Member::IMemberT::MemberAccess a = Member::IMemberT::MemberAccess::member_public,
			Member::IMemberT::MemberVisibility v = Member::IMemberT::MemberVisibility::member_nothidden)
		{
			memberMap.insert(MemberMapT::value_type(name, StaticMemberFunction(func, a, v)));
		}

		/// @brief Add a (non-static) member function interface
		/// @param name member function name keyword
		/// @param func std::function<> accepting smart-pointer to contained object, and MEX arguments passed to member function during execution
		/// @param a function access flag (e.g. member_protected, member_public,...)
		/// @param v function visibility flag (member_hidden, member_nothidden)
		virtual void addFunction(std::string name, MEX_F func,
			Member::IMemberT::MemberAccess a = Member::IMemberT::MemberAccess::member_public,
			Member::IMemberT::MemberVisibility v = Member::IMemberT::MemberVisibility::member_nothidden)
		{
			memberMap.insert(MemberMapT::value_type(name, NonStaticMemberFunction(this,func, a, v)));
		}

		/// @brief Add static member variable interface
		/// @param name member variable name keyword
		/// @param get_func function accepting MEX arguments, used to retrieve variable value
		/// @param set_func function accepting MEX arguments, used to set variable value
		/// @param a function access flag (e.g. member_protected, member_public,...)
		/// @param v function visibility flag (member_hidden, member_nothidden)
		virtual void addStaticVariable(std::string name, MEX_SF_noRHS get_func, MEX_SF set_func = nullptr,
			Member::IMemberT::MemberAccess a = Member::IMemberT::MemberAccess::member_public,
			Member::IMemberT::MemberVisibility v = Member::IMemberT::MemberVisibility::member_nothidden)
		{
			memberMap.insert(MemberMapT::value_type(name, StaticMemberVariable(get_func, set_func, a, v)));
		}

		/// @brief Add (non-static) member variable interface
		/// @param name member variable name keyword
		/// @param get_func function accepting smart-pointer to contained object, and MEX arguments, used to retrieve variable value
		/// @param set_func function accepting smart-pointer to contained object, and MEX arguments, used to set variable value
		/// @param a function access flag (e.g. member_protected, member_public,...)
		/// @param v function visibility flag (member_hidden, member_nothidden)
		virtual void addVariable(std::string name, MEX_F_noRHS get_func, MEX_F set_func = nullptr,
			Member::IMemberT::MemberAccess a = Member::IMemberT::MemberAccess::member_public,
			Member::IMemberT::MemberVisibility v = Member::IMemberT::MemberVisibility::member_nothidden)
		{
			memberMap.insert(MemberMapT::value_type(name, NonStaticMemberVariable(this,get_func, set_func, a, v)));
		}

		/// Exception handler for errors thrown when executing a method from matlab
		void handle_exception(std::exception_ptr eptr, std::string functionName) {
			using namespace std;
			try {
				if (eptr) {
					rethrow_exception(eptr);
				}
			}
			catch (const exception& e) {
				std::string source;
				source += string("mexInterfaceManager:") + functionName;
				mexErrMsgIdAndTxt(source.c_str(), "mexInterfaceManager<%s>::%s\nCaught exception: %s", typeid(ObjType).name(), functionName.c_str(), e.what());
			}
		}

		/// helper function for getting object instance from mxArray* holding int64_ptr
		/// Returns a shared pointer to the object instance
		std::shared_ptr<ObjType> getObjectPtr(int nrhs, const mxArray* prhs[]) {
			if (nrhs < 1) {
				throw(extras::stacktrace_error("requires intptr argument specifying object to destruct"));
			}
			return ObjectManager<ObjType>::get(prhs[0]);
		}

		void clearObjects(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
#ifdef _DEBUG
			mexPrintf("Clearing All Objects.\n");
			mexEvalString("pause(0.2)");
#endif
			ObjectManager<ObjType>::clearObjects();
		}


		void getMembers(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) const {
			extras::cmex::MxStruct outStruct(memberMap.size(), { "Name","Type","Static","Hidden","Access" });
			size_t k = 0;

			for (auto& f : memberMap) {
				outStruct(k, "Name") = f.first.c_str();
				if (f.second.type() == Member::IMemberT::MemberType::member_function) {
					outStruct(k, "Type") = "method";
				}
				else {
					outStruct(k, "Type") = "property";
				}

				outStruct(k, "Static") = mxCreateLogicalScalar(f.second.isStatic());
				outStruct(k, "Hidden") = mxCreateLogicalScalar(f.second.isHidden());

				switch (f.second.access()) {
				case Member::IMemberT::MemberAccess::member_public:
					outStruct(k, "Access") = "public";
					break;
				case Member::IMemberT::MemberAccess::member_protected:
					outStruct(k, "Access") = "public";
					break;
				case Member::IMemberT::MemberAccess::member_private:
					outStruct(k, "Access") = "private";
					break;
				}
			}

			plhs[0] = outStruct;
		}

	public:

		virtual ~mexObjectInterface() {
#ifdef _DEBUG
			mexPrintf("Destroying mexInterfaceManager<%s,...>\n", typeid(ObjType).name());
			mexEvalString("pause(0.2)");
#endif
		}

		mexObjectInterface() {
			using namespace std::placeholders;

			addStaticFunction("new", std::bind(&mexObjectInterface::new_object, this, _1, _2, _3, _4));  //add 'new' command
			addStaticFunction("delete", std::bind(&mexObjectInterface::delete_object, this, _1, _2, _3, _4));  //add 'delete' command
			addStaticFunction("clear_mex_objects", std::bind(&mexObjectInterface::clearObjects, this, _1, _2, _3, _4));  //add 'clear_mex_objects' command
			addStaticFunction("getMembers", std::bind(&mexObjectInterface::getMembers, this, _1, _2, _3, _4)); //return cellstr specifying method names
		}

		void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
			/// validate first arg
			if (nrhs < 1 || !mxIsChar(prhs[0])) {
				mexErrMsgIdAndTxt("mexInterfaceManager:argumentError", "Invalid argument: missing method name.");
			}
			std::string funcName;
			try {
				funcName = extras::cmex::getstring(prhs[0]);

				size_t l = funcName.find('.');
				if (l != std::string::npos) {//Found Property, use set/get
					std::string propName = funcName.substr(0, l);
					std::string propMethod = funcName.substr(l + 1, std::string::npos);
					propMethod = extras::tolower(propMethod);

					//verify property method is get or set
					if (strcmp(propMethod.c_str(), "get") != 0 || strcmp(propMethod.c_str(), "set") != 0) {
						throw(extras::stacktrace_error(
							std::string("'") + funcName + std::string("' is not a valid property and set/get method.\nSyntax should be: ") + propName + std::string(".get/set")
						));
					}


					auto search = memberMap.find(propName);
					if (search != memberMap.end()) {
						Member::IMemberVariableT& thisProp = dynamic_cast<Member::IMemberVariableT&>(search->second);
							if (strcmp(propMethod.c_str(), "get") == 0) {//get
								thisProp.get(nlhs, plhs, nrhs - 1, nrhs > 1 ? &(prhs[1]) : nullptr);
							}
							else { //set
								if (!thisProp.isSettable()) {
									throw(extras::stacktrace_error(
										std::string("'") + propName + std::string("' is not settable (set method not defined in c++ code).")
									));
								}
								thisProp.set(nlhs, plhs, nrhs - 1, nrhs > 1 ? &(prhs[1]) : nullptr);
							}
					}
					else {

						throw(extras::stacktrace_error(
							std::string("'") + propName + std::string("' property not found.")
						));
					}

				}
				else {//use method lookup
					auto search = memberMap.find(funcName);
					if (search != memberMap.end()) {
						Member::IMemberFunctionT& thisFunc = dynamic_cast<Member::IMemberFunctionT&>(search->second);
						thisFunc(nlhs, plhs, nrhs - 1, &(prhs[1])); //execute command
					}
					else {
						throw(extras::stacktrace_error(
							std::string("'") + funcName + std::string("' method not found.")));
					}
				}
			}
			catch (...) {
				handle_exception(std::current_exception(), funcName);
			}
		}
	};

 }}