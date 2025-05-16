# FreeSASA - Rosetta 統合マスタードキュメント

## 1. プロジェクト概要

### 1.1 目的

RosettaフレームワークにFreeSASA（Fast, Exact Solvent Accessible Surface Area）ライブラリを統合し、分子表面積計算機能を追加する。FreeSASAは軽量でポータブルなSASA計算ライブラリであり、Rosettaのアプリケーションやプロトコルで直接利用可能にする。

### 1.2 背景

タンパク質表面の特性（特に溶媒接触表面積）は、タンパク質の安定性や相互作用性などを理解する上で重要な指標である。現在、Rosettaは独自のSASA計算方法を持っているが、FreeSASAのような専用ライブラリを統合することで、以下の利点が期待できる：

- 計算精度と速度の向上
- 複数のアルゴリズム（Lee-Richards/Shrake-Rupley）の選択肢
- 標準化されたSASA計算へのアクセス
- タンパク質設計や解析パイプラインでの活用

### 1.3 プロジェクト成果物

1. Rosettaにビルドインされた状態のFreeSASA統合
2. Rosettaのコード規約に準拠したインターフェースクラス
3. 単体テストとサンプルアプリケーション
4. 使用方法とAPIドキュメント

## 2. コーディングルール

### 2.1 スタイルと命名規則

1. **括弧内のスペース**: 常に括弧の内側にスペースを入れること
   ```cpp
   // 正しい例
   void function( int parameter );
   if ( condition ) {
     // コード
   }
   
   // 誤った例
   void function(int parameter);
   if (condition) {
     // コード
   }
   ```

2. **クラス名**: CamelCase形式を使用
   ```cpp
   class FreeSASA {};
   ```

3. **メソッド名**: camelCase形式を使用
   ```cpp
   void calculateSASA();
   ```

4. **変数名**: snake_case形式を使用
   ```cpp
   float sasa_value;
   ```

5. **定数**: 大文字のSNAKE_CASE形式を使用
   ```cpp
   const float DEFAULT_PROBE_RADIUS = 1.4;
   ```

6. **Rosetta型の省略形**: 以下のRosettaの型に対してはtypedefを使った省略形を使用する
   ```cpp
   // 長い形式                        // 省略形（推奨）
   std::string                        // string
   core::Real                         // Real
   core::Size                         // Size
   core::pose::Pose const &           // Pose const &
   core::id::AtomID                   // AtomID
   core::conformation::Residue        // Residue
   utility::vector1< core::Size >     // VecSize
   utility::vector1< core::Real >     // VecReal
   utility::vector1< std::string >    // VecString
   
   // 正しい例
   Real calculate_sasa( Pose const & pose, Size residue_index );
   string get_residue_name( Size residue_index );
   std::map< AtomID, Real > get_atom_sasa_map();
   VecReal get_sasa_values();
   VecSize get_exposed_residues( Real cutoff );
   VecString get_residue_names( Pose const & pose );
   
   // 誤った例（冗長）
   core::Real calculate_sasa( core::pose::Pose const & pose, core::Size residue_index );
   std::string get_residue_name( core::Size residue_index );
   std::map< core::id::AtomID, core::Real > get_atom_sasa_map();
   utility::vector1< core::Real > get_sasa_values();
   utility::vector1< core::Size > get_exposed_residues( Real cutoff );
   utility::vector1< std::string > get_residue_names( Pose const & pose );
   ```

7. **インクルードガード**: 以下の形式を使用
   ```cpp
   #ifndef INCLUDED_core_scoring_sasa_FreeSASA_hh
   #define INCLUDED_core_scoring_sasa_FreeSASA_hh
   
   // コード
   
   #endif // INCLUDED_core_scoring_sasa_FreeSASA_hh
   ```

7. **条件付きコンパイル**: 一貫した`USE_FREESASA`マクロを使用
   ```cpp
   #ifdef USE_FREESASA
   // FreeSASA依存コード
   #else
   // 代替実装
   #endif
   ```

8. **コメント**: Doxygen形式のコメントを使用
   ```cpp
   /// @brief すべての原子のSASAを計算
   /// @param pose SASAを計算するポーズ
   /// @return 原子IDからSASA値へのマップ
   std::map< core::id::AtomID, core::Real > 
   calculateAtomSASA( core::pose::Pose const & pose );
   ```

9. **名前空間**: Rosettaの名前空間階層に従う
   ```cpp
   namespace core {
   namespace scoring {
   namespace sasa {
   
   // FreeSASAコード
   
   } // namespace sasa
   } // namespace scoring
   } // namespace core
   ```

10. **エラー処理**: Rosettaのエラー処理メカニズムを使用
    ```cpp
    utility::excn::EXCN_Msg_Exception( "FreeSASA calculation failed" );
    ```

### 2.2 ファイル構成

1. FreeSASAインターフェースヘッダの配置場所:
   ```
   /Users/nobuyasu/dev_2025/rosetta/source/src/core/scoring/sasa/FreeSASA.hh
   ```

2. FreeSASAインターフェース実装の配置場所:
   ```
   /Users/nobuyasu/dev_2025/rosetta/source/src/core/scoring/sasa/FreeSASA.cc
   ```

3. FreeSASA外部ライブラリ設定の配置場所:
   ```
   /Users/nobuyasu/dev_2025/rosetta/source/external/freesasa.external.settings
   ```

## 3. プロジェクト計画

### 3.1 開発ブランチ

```bash
# 現在のブランチから新しいブランチを作成
git checkout -b add_freesasa_lib
```

名前: `add_freesasa_lib`  
説明: FreeSASA統合のための開発ブランチ

### 3.2 統合フェーズとタスク

#### フェーズ1: 準備作業とFreeSASAのRosettaへの組み込み

1. **FreeSASAソースコードの配置**
   ```bash
   # FreeSASAのソースコードをRosettaの外部ライブラリディレクトリにコピー
   mkdir -p /Users/nobuyasu/dev_2025/rosetta/source/external/freesasa
   cp -r /Users/nobuyasu/dev_2025/rosetta/mytest/exposed_hydrophobics/freesasa/* /Users/nobuyasu/dev_2025/rosetta/source/external/freesasa/
   
   # ライブラリディレクトリの作成
   mkdir -p /Users/nobuyasu/dev_2025/rosetta/source/external/freesasa/lib
   cp /Users/nobuyasu/dev_2025/rosetta/mytest/exposed_hydrophobics/lib/libfreesasa.a /Users/nobuyasu/dev_2025/rosetta/source/external/freesasa/lib/
   ```

2. **FreeSASA設定ファイルの作成**
   ```bash
   # 外部ライブラリ設定ファイルを作成
   touch /Users/nobuyasu/dev_2025/rosetta/source/external/freesasa.external.settings
   ```
   
   設定ファイルの内容:
   ```python
   # FreeSASA external library settings
   import os
   
   def settings(env):
       env.Append(CPPDEFINES=["USE_FREESASA"])
       env.Append(CPPPATH=[os.path.join(env["EXTERNAL_TOOLS_PATH"],"freesasa/src")])
       env.Append(LIBPATH=[os.path.join(env["EXTERNAL_TOOLS_PATH"],"freesasa/lib")])
       env.Append(LIBS=["freesasa"])
       return True
   ```

3. **SConscript.externalの更新**
   
   ```bash
   # SConscript.externalファイルを編集
   vim /Users/nobuyasu/dev_2025/rosetta/source/external/SConscript.external
   ```
   
   追加する内容:
   ```python
   # FreeSASA
   if env.get('use_freesasa', False):
       freesasa_success = env.get('FREESASA_SUCCESS', False)
       if freesasa_success or SConscript('freesasa.external.settings', exports=['env']):
           env['FREESASA_SUCCESS'] = True
   ```

4. **CMake設定の更新**
   
   ```bash
   # CMakeの設定ファイルを編集
   vim /Users/nobuyasu/dev_2025/rosetta/source/cmake/build_settings/all_lib_settings.cmake
   ```
   
   追加する内容:
   ```cmake
   # FreeSASA Library
   option(USE_FREESASA "Enable FreeSASA support" ON)
   if(USE_FREESASA)
     message(STATUS "Building with FreeSASA support")
     add_definitions(-DUSE_FREESASA)
     include_directories(${CMAKE_SOURCE_DIR}/external/freesasa/src)
     link_directories(${CMAKE_SOURCE_DIR}/external/freesasa/lib)
   endif()
   ```

#### フェーズ2: インターフェースクラスの設計と実装

1. **コアディレクトリ構造の作成**
   
   ```bash
   # インターフェースクラス用のディレクトリを作成
   mkdir -p /Users/nobuyasu/dev_2025/rosetta/source/src/core/scoring/sasa
   ```

2. **ヘッダーファイルの作成**
   
   ```bash
   # FreeSASAインターフェースヘッダを作成
   touch /Users/nobuyasu/dev_2025/rosetta/source/src/core/scoring/sasa/FreeSASA.hh
   ```

3. **実装ファイルの作成**
   
   ```bash
   # FreeSASAインターフェース実装を作成
   touch /Users/nobuyasu/dev_2025/rosetta/source/src/core/scoring/sasa/FreeSASA.cc
   ```

4. **src.settings ファイルの更新**
   
   ```bash
   # src.settingsファイルを編集 (core.3.src.settingsが最適)
   vim /Users/nobuyasu/dev_2025/rosetta/source/src/core.3.src.settings
   ```
   
   追加する内容:
   ```
   core/scoring/sasa
   ```

#### フェーズ3: テストとドキュメントの作成

1. **単体テストの作成**
   
   ```bash
   # テストディレクトリの作成
   mkdir -p /Users/nobuyasu/dev_2025/rosetta/source/test/core/scoring/sasa
   
   # テストファイルの作成
   touch /Users/nobuyasu/dev_2025/rosetta/source/test/core/scoring/sasa/FreeSASATest.cxxtest.hh
   ```

2. **パイロットアプリケーションの作成**
   
   ```bash
   # アプリケーションディレクトリの作成
   mkdir -p /Users/nobuyasu/dev_2025/rosetta/source/src/apps/pilot/nobuyasu
   
   # アプリケーションファイルの作成
   touch /Users/nobuyasu/dev_2025/rosetta/source/src/apps/pilot/nobuyasu/test_freesasa.cc
   ```

3. **ドキュメントの作成**
   
   ```bash
   # READMEファイルの作成
   touch /Users/nobuyasu/dev_2025/rosetta/source/src/core/scoring/sasa/README.md
   ```

#### フェーズ4: Xcodeプロジェクト設定

1. **Xcodeプロジェクト生成**

   ```bash
   # Xcodeプロジェクト生成
   cd /Users/nobuyasu/dev_2025/rosetta/source/xcode
   python make_project.py --extras=freesasa
   ```

2. **Xcodeプロジェクト設定の微調整**

   - Xcodeでプロジェクトを開く
   - ヘッダ検索パスに`/Users/nobuyasu/dev_2025/rosetta/source/external/freesasa/src`を追加
   - ライブラリ検索パスに`/Users/nobuyasu/dev_2025/rosetta/source/external/freesasa/lib`を追加
   - `libfreesasa.a`をプロジェクトにリンク

#### フェーズ5: ビルドとテスト

1. **コマンドラインでのビルド**

   ```bash
   # SCons を使用したビルド
   cd /Users/nobuyasu/dev_2025/rosetta/source
   ./scons.py -j4 mode=release extras=freesasa bin
   
   # CMake を使用したビルド
   cd /Users/nobuyasu/dev_2025/rosetta/source/cmake
   mkdir -p build_freesasa && cd build_freesasa
   cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_FREESASA=ON
   make -j4
   ```

2. **テストの実行**

   ```bash
   # SCons を使用したテスト
   cd /Users/nobuyasu/dev_2025/rosetta/source
   ./scons.py -j4 mode=release extras=freesasa cat=core.scoring.sasa
   
   # CMake を使用したテスト
   cd /Users/nobuyasu/dev_2025/rosetta/source/cmake/build_freesasa
   ctest -R FreeSASATest
   ```

## 4. インターフェース仕様

### 4.1 クラス図

```
+------------------------------------------+
| FreeSASA                                 |
+------------------------------------------+
| - probe_radius_: Real                    |
| - algorithm_: int                        |
| - resolution_: int                       |
+------------------------------------------+
| + FreeSASA( probe_radius: Real = 1.4 )   |
| + ~FreeSASA()                            |
| + calculateAtomSASA( pose: Pose ): vector1 |
| + calculateResidueSASA( pose: Pose ): map  |
| + getExposedResidues( pose: Pose ): VecSize |
| + getSASAValues( pose: Pose ): VecReal   |
| + setAlgorithm( alg: int, res: int )     |
| + setProbeRadius( radius: Real )         |
| - createStructure( pose: Pose ): void*   |
+------------------------------------------+
```

### 4.2 転送宣言ファイル (FreeSASA.fwd.hh)

```cpp
// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the
// (c) Rosetta Commons. For more information, see http://www.rosettacommons.org.
// (c) Questions about this can be addressed to University of Washington UW TechTransfer,
// (c) email: license@u.washington.edu.

/// @file core/scoring/sasa/FreeSASA.fwd.hh
/// @brief Forward declarations for FreeSASA class
/// @author Nobuyasu Koga (nkoga@protein.osaka-u.ac.jp)

#ifndef INCLUDED_core_scoring_sasa_FreeSASA_fwd_hh
#define INCLUDED_core_scoring_sasa_FreeSASA_fwd_hh

// Utility Headers
#include <utility/pointer/owning_ptr.hh>

namespace core {
namespace scoring {
namespace sasa {

// Forward declaration of FreeSASA class
class FreeSASA;

// Owning pointer typedef
typedef utility::pointer::shared_ptr< FreeSASA > FreeSASAOP;
typedef utility::pointer::shared_ptr< FreeSASA const > FreeSASACOP;

} // namespace sasa
} // namespace scoring
} // namespace core

#endif // INCLUDED_core_scoring_sasa_FreeSASA_fwd_hh
```

### 4.3 ヘッダーファイル (FreeSASA.hh)

```cpp
// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the
// (c) Rosetta Commons. For more information, see http://www.rosettacommons.org.
// (c) Questions about this can be addressed to University of Washington UW TechTransfer,
// (c) email: license@u.washington.edu.

/// @file core/scoring/sasa/FreeSASA.hh
/// @brief FreeSASA library integration for SASA calculations
/// @author Nobuyasu Koga (nkoga@protein.osaka-u.ac.jp)

#ifndef INCLUDED_core_scoring_sasa_FreeSASA_hh
#define INCLUDED_core_scoring_sasa_FreeSASA_hh

// Package headers
#include <core/types.hh>
#include <core/pose/Pose.fwd.hh>
#include <core/id/AtomID.fwd.hh>
#include <utility/vector1.hh>
#include <core/scoring/sasa/FreeSASA.fwd.hh>

// C++ headers
#include <map>
#include <string>

// Forward declarations for FreeSASA
struct freesasa_structure;
struct freesasa_result;
struct freesasa_parameters;

namespace core {
namespace scoring {
namespace sasa {

/// @brief Interface to FreeSASA library for SASA calculations
///
/// This class provides an interface to the FreeSASA library 
/// (https://freesasa.github.io/) for calculating solvent 
/// accessible surface area (SASA) of proteins.
///
/// FreeSASA implements two algorithms for SASA calculation:
/// 1. Lee-Richards algorithm: More accurate but slower
/// 2. Shrake-Rupley algorithm: Faster but less accurate
///
/// The class allows users to:
/// - Calculate SASA for all atoms in a pose
/// - Calculate SASA for all residues in a pose
/// - Identify exposed residues based on SASA cutoffs
/// - Configure probe radius and algorithm parameters
///
/// Usage example:
/// @code
///   FreeSASA sasa_calc;
///   std::map<Size, Real> residue_sasa = sasa_calc.calculateResidueSASA( pose );
///   VecSize exposed_residues = sasa_calc.getExposedResidues( pose, 20.0 );
/// @endcode
///
/// @note This integration uses FreeSASA version 2.1.2
/// @see https://freesasa.github.io/doxygen/ for FreeSASA API documentation
class FreeSASA {

public:

  /// @brief Constructor with default parameters
  /// @param probe_radius The probe radius in Angstroms ( default 1.4 )
  FreeSASA( Real probe_radius = 1.4 );
  
  /// @brief Destructor
  ~FreeSASA();
  
  /// @brief Static factory method to create FreeSASA object with Owning Pointer
  /// @param probe_radius The probe radius in Angstroms ( default 1.4 )
  /// @return FreeSASAOP (shared_ptr to FreeSASA)
  static FreeSASAOP 
  create_sasa_calculator( Real probe_radius = 1.4 ) {
    return utility::pointer::make_shared< FreeSASA >( probe_radius );
  }
  
  /// @brief Calculate SASA for all atoms in a pose
  /// @param pose The pose to calculate SASA for
  /// @return A map of atom IDs to SASA values
  std::map< AtomID, Real > 
  calculateAtomSASA( Pose const & pose );
  
  /// @brief Calculate SASA for all residues in a pose
  /// @param pose The pose to calculate SASA for
  /// @return A map of residue indices to SASA values
  std::map< Size, Real > 
  calculateResidueSASA( Pose const & pose );
  
  /// @brief Get residues with SASA above a cutoff value
  /// @param pose The pose to analyze
  /// @param cutoff_value The SASA cutoff value in Å²
  /// @return Vector of residue indices with SASA above cutoff
  VecSize
  getExposedResidues( Pose const & pose, Real cutoff_value = 20.0 );
  
  /// @brief Get SASA values as a vector
  /// @param pose The pose to calculate SASA for
  /// @return Vector of SASA values for each residue
  VecReal
  getSASAValues( Pose const & pose );
  
  /// @brief Set the algorithm for SASA calculation
  /// @param algorithm 0 for Lee-Richards, 1 for Shrake-Rupley
  /// @param resolution Number of slices for Lee-Richards or test points for Shrake-Rupley
  void setAlgorithm( int algorithm, int resolution = 20 );
  
  /// @brief Set the probe radius
  /// @param radius The probe radius in Angstroms
  void setProbeRadius( Real radius );
  
private:

  /// @brief Create a FreeSASA structure from a Rosetta pose
  /// @param pose The pose to convert
  /// @param skip_hydrogens Whether to skip hydrogen atoms
  /// @return A pointer to the created FreeSASA structure ( caller must free )
  freesasa_structure* createStructureFromPose(
    Pose const & pose,
    bool skip_hydrogens = true );
  
  /// @brief Initialize default parameters
  void initParameters();
  
  /// @brief Determine element from atom name
  /// @param atom_name The atom name
  /// @return The element symbol
  string getAtomElement( string const & atom_name );
  
  // Class members
  Real probe_radius_;
  int algorithm_;      // 0 = Lee-Richards, 1 = Shrake-Rupley
  int resolution_;     // Slices or test points
  bool initialized_;
};

} // namespace sasa
} // namespace scoring
} // namespace core

#endif // INCLUDED_core_scoring_sasa_FreeSASA_hh
```

### 4.3 実装ファイル (FreeSASA.cc)

```cpp
// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the
// (c) Rosetta Commons. For more information, see http://www.rosettacommons.org.
// (c) Questions about this can be addressed to University of Washington UW TechTransfer,
// (c) email: license@u.washington.edu.

/// @file core/scoring/sasa/FreeSASA.cc
/// @brief Implementation of FreeSASA interface
/// @author Nobuyasu Koga (nkoga@protein.osaka-u.ac.jp)

// Unit headers
#include <core/scoring/sasa/FreeSASA.hh>

// Package headers
#include <core/pose/Pose.hh>
#include <core/conformation/Residue.hh>
#include <core/id/AtomID.hh>
#include <basic/Tracer.hh>

// External headers
#ifdef USE_FREESASA
#include <freesasa.h>
#endif

// C++ headers
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>

namespace core {
namespace scoring {
namespace sasa {

static basic::Tracer TR("core.scoring.sasa.FreeSASA");

// Constructor implementation
FreeSASA::FreeSASA( core::Real probe_radius ) :
  probe_radius_( probe_radius ),
  algorithm_( 0 ),        // Lee-Richards by default
  resolution_( 20 ),
  initialized_( false )
{
#ifdef USE_FREESASA
  // Set FreeSASA to silent mode
  freesasa_set_verbosity( FREESASA_V_SILENT );
  initialized_ = true;
#else
  TR.Warning << "FreeSASA support not compiled in. SASA calculations will fail." << std::endl;
  initialized_ = false;
#endif
}

// その他の実装内容...
// (実装の詳細部分は省略)

} // namespace sasa
} // namespace scoring
} // namespace core
```

## 5. テストとパイロットアプリケーション

### 5.1 単体テスト (FreeSASATest.cxxtest.hh)

```cpp
// 単体テスト例
#include <cxxtest/TestSuite.h>
#include <core/scoring/sasa/FreeSASA.hh>
#include <core/pose/Pose.hh>
#include <core/import_pose/import_pose.hh>
#include <core/conformation/Residue.hh>

class FreeSASATests : public CxxTest::TestSuite {
public:
  void test_basic_functionality() {
    // 簡単なポーズを作成してテスト
    core::pose::Pose pose;
    core::import_pose::pose_from_file( pose, "test_data/2GB1.pdb" );
    
    core::scoring::sasa::FreeSASA sasa_calc;
    std::map<core::Size, core::Real> rsd_sasa = sasa_calc.calculateResidueSASA( pose );
    
    // 例：各残基のSASAが0以上であることを確認
    for ( auto const & pair : rsd_sasa ) {
      TS_ASSERT_LESS_THAN_EQUALS( 0.0, pair.second );
    }
    
    // 例：アルゴリズム変更が機能することを確認
    sasa_calc.setAlgorithm( 1, 100 );  // Shrake-Rupley
    std::map<core::Size, core::Real> rsd_sasa2 = sasa_calc.calculateResidueSASA( pose );
    TS_ASSERT( !rsd_sasa2.empty() );
  }
};
```

### 5.2 パイロットアプリケーション (test_freesasa.cc)

```cpp
// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// This file is part of the Rosetta software suite and is made available under license.
// The Rosetta software is developed by the contributing members of the Rosetta Commons consortium.
// (c) For more information, see http://www.rosettacommons.org.

/// @file /src/apps/pilot/nobuyasu/test_freesasa.cc
/// @brief Test application for FreeSASA integration
/// @author Nobuyasu Koga (nkoga@protein.osaka-u.ac.jp)

// Rosetta headers
#include <core/init.hh>
#include <core/pose/Pose.hh>
#include <core/scoring/sasa/FreeSASA.hh>
#include <core/import_pose/import_pose.hh>
#include <basic/options/option.hh>
#include <basic/options/keys/in.OptionKeys.gen.hh>
#include <basic/Tracer.hh>

// C++ headers
#include <iostream>
#include <iomanip>
#include <fstream>

static basic::Tracer TR("test_freesasa");

using namespace core;

int main( int argc, char *argv[] ) {
  try {
    // Initialize Rosetta
    init( argc, argv );
    
    using namespace basic::options;
    using namespace basic::options::OptionKeys;
    
    // Check if input PDB file was provided
    if ( !option[in::file::s].user() ) {
      TR.Error << "No input PDB file provided. Use -in:file:s to specify." << std::endl;
      return 1;
    }
    
    // Load pose from PDB file
    pose::Pose pose;
    import_pose::pose_from_file( pose, option[in::file::s]()[1] );
    TR.Info << "Loaded pose with " << pose.size() << " residues." << std::endl;
    
    // Create FreeSASA calculator with default parameters
    scoring::sasa::FreeSASA sasa_calc;
    
    // Calculate per-residue SASA
    std::map<Size, Real> residue_sasa = sasa_calc.calculateResidueSASA( pose );
    
    // Calculate per-atom SASA
    std::map<id::AtomID, Real> atom_sasa = sasa_calc.calculateAtomSASA( pose );
    
    // Output results to file
    std::ofstream out_file( "/Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib/sasa_results.txt" );
    
    // Per-residue SASA
    out_file << "# Per-residue SASA values:" << std::endl;
    out_file << "#   Residue   SASA (Å²)" << std::endl;
    for ( Size i = 1; i <= pose.size(); ++i ) {
      string res_name = pose.residue( i ).name3();
      out_file << std::setw( 8 ) << i << std::setw( 5 ) << res_name 
               << std::setw( 12 ) << std::fixed << std::setprecision( 2 ) << residue_sasa[i] << std::endl;
    }
    
    // Print atom count and total SASA
    TR.Info << "Calculated SASA for " << atom_sasa.size() << " atoms." << std::endl;
    
    // Get exposed residues using Vector1型
    Real cutoff_value = 20.0; // 基準値: 20.0 Å²
    VecSize exposed_residues = sasa_calc.getExposedResidues( pose, cutoff_value );
    
    TR.Info << "Residues with SASA > " << cutoff_value << " Å²: " << exposed_residues.size() << std::endl;
    out_file << "# Exposed residues (SASA > " << cutoff_value << " Å²):" << std::endl;
    for ( Size i = 1; i <= exposed_residues.size(); ++i ) {
      Size res_num = exposed_residues[i];
      string res_name = pose.residue( res_num ).name3();
      out_file << std::setw( 8 ) << res_num << std::setw( 5 ) << res_name 
               << std::setw( 12 ) << std::fixed << std::setprecision( 2 ) << residue_sasa[res_num] << std::endl;
    }
    
    // SASAをVector1として取得
    VecReal sasa_values = sasa_calc.getSASAValues( pose );
    
    // Calculate total SASA
    Real total_sasa = 0.0;
    for ( Size i = 1; i <= sasa_values.size(); ++i ) {
      total_sasa += sasa_values[i];
    }
    
    TR.Info << "Total SASA: " << total_sasa << " Å²" << std::endl;
    out_file << "# Total SASA: " << total_sasa << " Å²" << std::endl;
    
    out_file.close();
    
    // 追加のテストコードは省略

    return 0;
  } catch ( utility::excn::Exception const & e ) {
    std::cerr << "Exception caught: " << e.msg() << std::endl;
    return 1;
  } catch ( std::exception const & e ) {
    std::cerr << "STL exception caught: " << e.what() << std::endl;
    return 1;
  } catch ( ... ) {
    std::cerr << "Unknown exception caught" << std::endl;
    return 1;
  }
}
```

## 6. Xcode開発とデバッグ

### 6.1 Xcodeプロジェクト設定

```bash
# Xcodeプロジェクト生成
cd /Users/nobuyasu/dev_2025/rosetta/source/xcode
python make_project.py --extras=freesasa
```

### 6.2 デバッグ設定

- Xcodeでプロジェクトを開く
- ナビゲータでクラスを検索して`FreeSASA.cc`を開く
- 以下の場所にブレークポイントを設定：
  - `FreeSASA`コンストラクタ
  - `calculateAtomSASA`メソッド内のFreeSASA構造体作成部分
  - `calculateResidueSASA`メソッド内のSASA合計計算部分

### 6.3 テストアプリケーションの実行設定

- スキーム選択: `test_freesasa`
- 引数: `-in:file:s <PDBファイルへのパス>`
- 作業ディレクトリ: `/Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib`

## 7. 引き継ぎ情報

### 7.1 主要ファイル

- **FreeSASAソース**: `/Users/nobuyasu/dev_2025/rosetta/mytest/exposed_hydrophobics/freesasa/`
- **FreeSASAヘッダ**: `/Users/nobuyasu/dev_2025/rosetta/mytest/exposed_hydrophobics/include/freesasa.h`
- **ライブラリファイル**: `/Users/nobuyasu/dev_2025/rosetta/mytest/exposed_hydrophobics/lib/libfreesasa.a`
- **計画書**: `/Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib/FREESASA_INTEGRATION_MASTER.md`

### 7.2 作業ディレクトリとテストコード

- **作業ディレクトリ**: 実装中に生成される一時ファイルやビルド結果は `/Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib/` に保存する
- **テストコード作成場所**: `/Users/nobuyasu/dev_2025/rosetta/source/src/apps/pilot/nobuyasu/` にテストコードを作成する

### 7.3 コミット計画

1. **初期セットアップ**:
   - FreeSASAライブラリの追加
   - ビルドスクリプトの更新

2. **インターフェース実装**:
   - コアクラスの実装
   - 基本機能のテスト

3. **テストとドキュメント**:
   - 単体テストの追加
   - ドキュメントの整備

4. **サンプルアプリケーション**:
   - 簡単なSASA計算アプリの提供

## 8. テスト結果の生成とレポート

### 8.1 テスト結果のHTML形式出力

テスト実行結果を視覚的に確認しやすいHTML形式でも出力するため、以下の機能を実装します：

1. **HTML結果レポート生成スクリプト**

```python
# /Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib/generate_html_report.py

import os
import sys
import json
import datetime
import matplotlib.pyplot as plt
from pathlib import Path

def generate_html_report( results_file, output_file ):
    """
    FreeSASAテスト結果からHTMLレポートを生成する
    
    Parameters:
    results_file (str): SASAテスト結果のテキストファイルパス
    output_file (str): 出力するHTMLファイルパス
    """
    # 結果ファイルの読み込み
    with open( results_file, 'r' ) as f:
        lines = f.readlines()
    
    # パース
    residue_data = []
    total_sasa = None
    
    for line in lines:
        line = line.strip()
        if line.startswith( '#' ):
            if "Total SASA:" in line:
                total_sasa = float( line.split( ":" )[1].strip().split()[0] )
            continue
        
        if line:
            parts = line.split()
            if len( parts ) >= 3:
                residue_data.append({
                    'residue_number': int( parts[0] ),
                    'residue_name': parts[1],
                    'sasa': float( parts[2] )
                })
    
    # グラフの生成（残基ごとのSASA値）
    plt.figure( figsize=( 10, 6 ) )
    residue_numbers = [d['residue_number'] for d in residue_data]
    sasa_values = [d['sasa'] for d in residue_data]
    plt.bar( residue_numbers, sasa_values )
    plt.xlabel( 'Residue Number' )
    plt.ylabel( 'SASA (Å²)' )
    plt.title( 'Per-Residue SASA Values' )
    
    # グラフを一時的に保存
    graph_file = os.path.join( os.path.dirname( output_file ), 'sasa_graph.png' )
    plt.savefig( graph_file )
    
    # HTMLの生成
    html_content = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <title>FreeSASA Test Results</title>
        <style>
            body {{ font-family: Arial, sans-serif; margin: 20px; }}
            h1, h2 {{ color: #2c3e50; }}
            table {{ border-collapse: collapse; width: 100%; margin-bottom: 20px; }}
            th, td {{ border: 1px solid #ddd; padding: 8px; text-align: left; }}
            th {{ background-color: #f2f2f2; }}
            tr:nth-child(even) {{ background-color: #f9f9f9; }}
            .graph {{ margin: 20px 0; }}
            .summary {{ font-size: 18px; margin: 20px 0; }}
            .timestamp {{ color: #7f8c8d; font-size: 14px; }}
        </style>
    </head>
    <body>
        <h1>FreeSASA Test Results</h1>
        <p class="timestamp">Generated on: {datetime.datetime.now().strftime( '%Y-%m-%d %H:%M:%S' )}</p>
        
        <div class="summary">
            <p><strong>Total SASA:</strong> {total_sasa} Å²</p>
            <p><strong>Total Residues:</strong> {len( residue_data )}</p>
        </div>
        
        <h2>Per-Residue SASA Values</h2>
        <div class="graph">
            <img src="sasa_graph.png" alt="SASA Graph" style="max-width: 100%;">
        </div>
        
        <h2>Detailed Results</h2>
        <table>
            <tr>
                <th>Residue Number</th>
                <th>Residue Name</th>
                <th>SASA (Å²)</th>
            </tr>
    """
    
    # テーブルに残基ごとのデータを追加
    for data in residue_data:
        html_content += f"""
            <tr>
                <td>{data['residue_number']}</td>
                <td>{data['residue_name']}</td>
                <td>{data['sasa']:.2f}</td>
            </tr>
        """
    
    html_content += """
        </table>
    </body>
    </html>
    """
    
    # HTMLファイルの保存
    with open( output_file, 'w' ) as f:
        f.write( html_content )
    
    print( f"HTML report generated: {output_file}" )
    print( f"Graph image saved: {graph_file}" )

if __name__ == "__main__":
    if len( sys.argv ) < 3:
        print( "Usage: python generate_html_report.py <results_file> <output_html_file>" )
        sys.exit( 1 )
    
    generate_html_report( sys.argv[1], sys.argv[2] )
```

2. **パイロットアプリケーションからHTMLレポート生成を呼び出す**

パイロットアプリケーション（test_freesasa.cc）にHTMLレポート生成機能を追加します：

```cpp
// パイロットアプリケーションのメイン関数内に追加
// ファイル出力後に以下を追加
out_file.close();

// HTMLレポートの生成
TR.Info << "Generating HTML report..." << std::endl;
std::string cmd = "python /Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib/generate_html_report.py "
                + std::string( "/Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib/sasa_results.txt" )
                + " " 
                + std::string( "/Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib/sasa_report.html" );
int ret = system( cmd.c_str() );
if ( ret == 0 ) {
  TR.Info << "HTML report generated successfully." << std::endl;
} else {
  TR.Warning << "HTML report generation failed with return code: " << ret << std::endl;
}
```

3. **実行と確認方法**

テスト結果のHTMLレポートは以下の手順で確認できます：

```bash
# テスト実行
cd /Users/nobuyasu/dev_2025/rosetta/source/cmake/build_release
./bin/test_freesasa -in:file:s /path/to/pdb/file.pdb

# HTMLレポートを確認
open /Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib/sasa_report.html
```

このHTMLレポートには以下の情報が含まれます：
- 総SASA値と残基数の概要
- 残基ごとのSASA値を示す棒グラフ
- 各残基の詳細なSASA値を示す表

### 8.2 比較テスト機能

異なるSASA計算アルゴリズム（Lee-RichardsとShrake-Rupley）の結果を比較するHTMLレポートも生成します：

```python
# 比較結果をHTMLレポートに追加する関数
def generate_comparison_report( results_lr, results_sr, output_file ):
    """
    Lee-RichardsとShrake-Rupleyアルゴリズムの結果を比較するHTMLレポートを生成
    
    Parameters:
    results_lr (dict): Lee-Richardsアルゴリズムの結果
    results_sr (dict): Shrake-Rupleyアルゴリズムの結果
    output_file (str): 出力HTMLファイル
    """
    # 実装の詳細は省略
    # 比較グラフと表を含むHTMLを生成
```

## 9. プロジェクト完了判断基準

### 9.1 必須要件
- FreeSASA インターフェースが正常にビルドできる
- 主要な機能 (原子・残基レベルのSASA計算) が正常に動作する
- 単体テストが通過する
- パイロットアプリケーションが正常に動作する
- テスト結果がHTML形式で出力される
- Xcode プロジェクトが正常に生成・ビルドできる

### 9.2 品質要件
- コード品質: Rosetta コーディング規約への準拠
- パフォーマンス: 既存の実装と同等以上
- ドキュメント: クラス・メソッドの詳細なドキュメント
- 使用例: サンプルコードと使用方法の説明
- レポート: 視覚的に理解しやすいHTML形式の結果レポート

### 9.3 チェックリスト
```
[ ] インターフェースクラスの実装
[ ] ビルドシステムへの統合
[ ] 単体テストの実装
[ ] パイロットアプリケーションの実装
[ ] HTML結果レポート機能の実装
[ ] Xcode プロジェクト生成
[ ] ドキュメント作成
[ ] コードレビュー
[ ] パフォーマンステスト
[ ] プラットフォーム互換性テスト
```

## 9. 参考リソース

### 9.1 FreeSASA関連

- **公式サイト**: https://freesasa.github.io/
- **API ドキュメント**: https://freesasa.github.io/doxygen/
- **GitHub リポジトリ**: https://github.com/mittinatten/freesasa

### 9.2 Rosetta関連

- **Rosetta デベロッパーガイド**: https://www.rosettacommons.org/docs/latest/development_documentation/
- **Xcode プロジェクト生成**: https://www.rosettacommons.org/docs/latest/development_documentation/IDEs/xcode
- **外部ライブラリ統合**: https://www.rosettacommons.org/docs/latest/development_documentation/build_system/external_libraries

## 10. FreeSASA インターフェースクラスの使用例

### 10.1 基本的な使用方法

```cpp
#include <core/scoring/sasa/FreeSASA.hh>
#include <core/pose/Pose.hh>

// FreeSASAオブジェクトを作成（デフォルトのプローブ半径 = 1.4Å = 水分子の半径相当）
core::scoring::sasa::FreeSASA sasa_calc;

// ポーズを読み込み
Pose pose;
core::import_pose::pose_from_file( pose, "input.pdb" );

// 個々の原子のSASAを計算
std::map< AtomID, Real > atom_sasa = sasa_calc.calculateAtomSASA( pose );

// 残基ごとのSASAを計算
std::map< Size, Real > residue_sasa = sasa_calc.calculateResidueSASA( pose );

// 特定の残基のSASAにアクセス（例: 10番目の残基）
Real rsd10_sasa = residue_sasa[10];

// アルゴリズムをShrake-Rupleyに変更（より高速だが近似的）
// 第1引数: 0 = Lee-Richards, 1 = Shrake-Rupley
// 第2引数: 精度（スライス数またはテストポイント数）
sasa_calc.setAlgorithm( 1, 100 );

// プローブ半径を変更（単位: Å）
sasa_calc.setProbeRadius( 1.2 );

// 新しい設定で再計算
residue_sasa = sasa_calc.calculateResidueSASA( pose );

// 露出した残基のインデックスを取得（SASA > 20.0 Å²）
VecSize exposed_residues = sasa_calc.getExposedResidues( pose, 20.0 );

// 全残基のSASA値をVectorとして取得
VecReal sasa_values = sasa_calc.getSASAValues( pose );
```

### 10.2 Owning Pointerを使用した例

```cpp
#include <core/scoring/sasa/FreeSASA.hh>
#include <core/pose/Pose.hh>
#include <utility/vector1.hh>
#include <protocols/moves/Mover.hh>

using core::scoring::sasa::FreeSASA;
using core::scoring::sasa::FreeSASAOP;

class SASABasedMover : public protocols::moves::Mover {

public:

  SASABasedMover() {
    // Owning Pointerを使ってFreeSASAオブジェクトを作成
    sasa_calculator_ = utility::pointer::make_shared< FreeSASA >( 1.4 );
    
    // アルゴリズムの設定
    sasa_calculator_->setAlgorithm( 0, 20 ); // Lee-Richards法、スライス数20
  }
  
  virtual void apply( Pose & pose ) override {
    // 溶媒露出表面積を計算
    std::map< Size, Real > residue_sasa = sasa_calculator_->calculateResidueSASA( pose );
    
    // 露出残基を特定
    VecSize exposed_residues = sasa_calculator_->getExposedResidues( pose, exposure_cutoff_ );
    
    // 露出残基に対して処理を実行
    for ( Size i = 1; i <= exposed_residues.size(); ++i ) {
      Size res_num = exposed_residues[i];
      // 残基に対する処理...
    }
  }
  
  // その他のメソッド...
  
private:

  FreeSASAOP sasa_calculator_;
  Real exposure_cutoff_ = 20.0;
};

// プロトコルで使用する例
void run_protocol( Pose & pose ) {
  // 静的関数でOwning Pointerを作成
  FreeSASAOP sasa_calc = FreeSASA::create_sasa_calculator();
  
  // SASAを計算して結果を使用
  std::map< Size, Real > sasa_results = sasa_calc->calculateResidueSASA( pose );
  
  // 複数のスレッドで共有して使用可能
  #pragma omp parallel for
  for ( Size i = 1; i <= pose.size(); ++i ) {
    // ここでsasa_calcを安全に使用できます
    // Owning Pointerは参照カウントによって管理されます
  }
  
  // 別の設定でインスタンスを作成
  FreeSASAOP sasa_calc2 = utility::pointer::make_shared< FreeSASA >( 1.2 );
  sasa_calc2->setAlgorithm( 1, 100 ); // Shrake-Rupley法
}
```