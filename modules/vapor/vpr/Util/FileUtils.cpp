#include <vpr/vprConfig.h>
#include <vpr/System.h>
#include <vpr/Util/FileUtils.h>


namespace vpr
{

std::string replaceEnvVars(const std::string str,
                           const std::string startStr, const std::string endStr)
{

   std::string ret_str(endStr);
   std::string::size_type subst_begin(0), subst_end(0), subst_len(0);

   while( (subst_begin = ret_str.find(startStr)) != std::string::npos)
   {
      std::string env_var, env_var_val;   // The environment variable and its value

      subst_end = ret_str.find(endStr);
      subst_len = subst_end - subst_begin;
      env_var = std::string(ret_str, subst_begin, subst_len);
      vpr::System::getenv(env_var, env_var_val);
      ret_str.replace(subst_begin, subst_len, env_var_val);
   }

   return ret_str;
}

std::string replaceEnvVars(const std::string str)
{
   std::string ret_str(str);

   ret_str = replaceEnvVars(ret_str, std::string("${"), std::string("}") );
   ret_str = replaceEnvVars(ret_str, std::string("$("), std::string(")") );

   return ret_str;
}

} // End of vpr namespace
