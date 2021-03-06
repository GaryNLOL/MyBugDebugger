#pragma once
#include <array>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace GraphiteLogger
{
    namespace __hidden__
    {
        template <int I> struct choice : choice<I+1> {};
        template <> struct choice<10> {};

        template <class T> std::string convertToString(T const& t);

        template <typename T>
        auto convertToStringHelper(T const& t, choice<0>) -> decltype(std::to_string(t))
        {
            return std::to_string(t);
        }
        template <class T>
        auto convertToStringHelper(T const& t, choice<1>) -> decltype(std::string(t))
        {
            return std::string(t);
        }
        template <class T, class A>
        std::string convertToStringHelper(std::vector<T,A> const& v, choice<2> )
        {
            if(v.empty())
                return "{}";
            if(convertToString(v[0]) == "Not representable")
                return "Not representable";
            std::string str;
            str += "{";
            for(unsigned int i = 0; i < v.size(); i++)
            {
                str += convertToString(v[i]);
                if(i != v.size() - 1)
                    str += ", ";
            }
            str += "}";
            std::ostringstream stream;
            return str;
        }
        template <class T>
        std::string convertToStringHelper(T* t, choice<3>)
        {
            if(t == nullptr)
                return "nullptr";
            std::stringstream ss;
            std::string str;
            ss << t;
            ss >> str;
            return str;
        }
        template<class T>
        std::string convertToStringHelper(T const& t, choice<4>)
        {
            std::ostringstream ss;
            ss << t;
            return ss.str();
        }
        template <class T>
        std::string convertToStringHelper(T const& t, choice<5>)
        {
            return "Not representable";
        }
        template <typename T>
        std::string convertToString(T const& t) {
            return convertToStringHelper(t, choice<0>{});
        }
    }
    #ifdef GRAPHITE_LOGGER__USE_DEFAULT_LOG_LEVELS
    namespace LogLevels
    {
        constexpr int emergency = 0;
        constexpr int alert = 1;
        constexpr int critical = 2;
        constexpr int error = 3;
        constexpr int warning = 4;
        constexpr int notice = 5;
        constexpr int info = 6;
        constexpr int debug = 7;
    }
    #endif
    using GraphiteLogger::__hidden__::convertToString;
    class Logger
    {
    private:
        std::string name;
        int log_level;
        std::vector<std::ostream*> outputs;
        template<typename First>
        static std::string getMessage(First first)
        {
            return convertToString(first);
        }
        template<typename First, typename... Args>
        static std::string getMessage(First first, Args... args)
        {
            return convertToString(first) + getMessage(args...);
        }
        class LoggerHelper
        {
        private:
            std::vector<std::ostream*>& outputs;
            bool do_ignore;
            template<typename T>
            std::string getMessage(T element)
            {
                return convertToString(element);
            }
        public:
            LoggerHelper(std::vector<std::ostream*>& new_outputs, bool new_do_ignore = false): outputs(new_outputs), do_ignore(new_do_ignore) {}
            template<typename T>
            LoggerHelper& operator<<(T element)
            {
                if(this->do_ignore)
                    return *this;
                std::string arrangedMessage = getMessage(element);
                for(unsigned int i = 0; i < this->outputs.size(); i++)
                    *(this->outputs[i]) << arrangedMessage;
                return *this;
            }
            ~LoggerHelper()
            {
                if(this->do_ignore)
                    return;
                for(unsigned int i = 0; i < this->outputs.size(); i++)
                    *this->outputs[i] << "\n";
            }
        };
    public:
        Logger(const std::string& new_name, int new_log_level, const std::vector<std::ostream*>& new_outputs = {}): name(new_name), log_level(new_log_level), outputs(new_outputs) {}
        void addOutput(std::ostream* stream)
        {
            this->outputs.push_back(stream);
        }
        void setLogLevel(int new_log_level)
        {
            this->log_level = new_log_level;
        }
        LoggerHelper operator[](int message_log_level)
        {
            if(this->log_level < message_log_level)
                return LoggerHelper(this->outputs,true);
            std::time_t now = std::time(0);
            std::string nowInStr = std::ctime(&now);
            std::string arrangedMessage = (std::string) "[" + nowInStr.substr(0,nowInStr.size()-1) + "] " + name + ": ";
            for(unsigned int i = 0; i < this->outputs.size(); i++)
                *(this->outputs[i]) << arrangedMessage;
            return LoggerHelper(this->outputs);
        }
    };
}
