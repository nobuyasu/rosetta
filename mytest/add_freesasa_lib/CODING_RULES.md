# FreeSASA Integration Coding Rules

This document outlines the coding rules and style guidelines for integrating FreeSASA into Rosetta.

## Code Style Rules

1. **Spaces in Parentheses**: Always add spaces inside parentheses
   ```cpp
   // Correct
   void function( int parameter );
   if ( condition ) {
     // code
   }
   
   // Incorrect
   void function(int parameter);
   if (condition) {
     // code
   }
   ```

2. **Class Naming**: Use CamelCase for class names
   ```cpp
   class FreeSASA {};
   ```

3. **Method Naming**: Use camelCase for method names
   ```cpp
   void calculateSASA();
   ```

4. **Variable Naming**: Use snake_case for variable names
   ```cpp
   float sasa_value;
   ```

5. **Constants**: Use all uppercase with underscores for constants
   ```cpp
   const float DEFAULT_PROBE_RADIUS = 1.4;
   ```

6. **Include Guards**: Use the following format for include guards
   ```cpp
   #ifndef INCLUDED_core_scoring_sasa_FreeSASA_hh
   #define INCLUDED_core_scoring_sasa_FreeSASA_hh
   
   // code
   
   #endif // INCLUDED_core_scoring_sasa_FreeSASA_hh
   ```

7. **Conditional Compilation**: Use a consistent `USE_FREESASA` macro for conditional compilation
   ```cpp
   #ifdef USE_FREESASA
   // FreeSASA-dependent code
   #else
   // Alternative implementation
   #endif
   ```

8. **Comments**: Use Doxygen-style comments for documentation
   ```cpp
   /// @brief Calculate SASA for all atoms in a pose
   /// @param pose The pose to calculate SASA for
   /// @return A map of atom IDs to SASA values
   std::map< core::id::AtomID, core::Real > 
   calculateAtomSASA( core::pose::Pose const & pose );
   ```

9. **Namespaces**: Follow Rosetta's namespace hierarchy
   ```cpp
   namespace core {
   namespace scoring {
   namespace sasa {
   
   // FreeSASA code
   
   } // namespace sasa
   } // namespace scoring
   } // namespace core
   ```

10. **Error Handling**: Use Rosetta's error handling mechanisms
    ```cpp
    utility::excn::EXCN_Msg_Exception( "FreeSASA calculation failed" );
    ```

## File Organization

1. Place FreeSASA interface headers in:
   ```
   /Users/nobuyasu/dev_2025/rosetta/source/src/core/scoring/sasa/FreeSASA.hh
   ```

2. Place FreeSASA interface implementation in:
   ```
   /Users/nobuyasu/dev_2025/rosetta/source/src/core/scoring/sasa/FreeSASA.cc
   ```

3. Place FreeSASA external library configuration in:
   ```
   /Users/nobuyasu/dev_2025/rosetta/source/external/freesasa.external.settings
   ```

## External Library Management

1. External FreeSASA library files should be placed in:
   ```
   /Users/nobuyasu/dev_2025/rosetta/source/external/freesasa/
   ```

2. Include both source and pre-built binaries for cross-platform compatibility

## Pilot Application

1. Create a simple pilot application for testing FreeSASA integration in:
   ```
   /Users/nobuyasu/dev_2025/rosetta/source/src/apps/pilot/nobuyasu/
   ```

2. Follow Rosetta's application structure and command-line parsing patterns

## Build System Integration

1. Update SCons build system files to include FreeSASA
2. Update CMake build system files to include FreeSASA
3. Configure for Xcode development environment

## Testing

1. Test FreeSASA interface with unit tests
2. Test FreeSASA integration with a simple pilot application
3. Ensure cross-platform compatibility