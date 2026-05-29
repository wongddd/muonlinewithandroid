import java.io.FileInputStream
import java.io.FileOutputStream
import java.util.Properties

plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android") version "1.9.22"
}

// Auto-increment build number (read from version.properties)
fun getNextBuildNumber(): Int {
    val propsFile = rootProject.file("../version.properties")
    val props = Properties()
    if (propsFile.exists()) {
        props.load(FileInputStream(propsFile))
    }
    val current = props.getProperty("build.number", "1").toInt()
    val next = current + 1
    props.setProperty("build.number", next.toString())
    props.store(FileOutputStream(propsFile), "Auto-incremented build version")
    return next
}

android {
    namespace = "com.mu.client"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.mu.client"
        minSdk = 26
        targetSdk = 34
        versionCode = getNextBuildNumber()
        versionName = "0.1.${versionCode}"

        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17"
                arguments += listOf(
                    "-DANDROID_STL=c++_shared",
                    "-DCMAKE_BUILD_TYPE=Release"
                )
                abiFilters += listOf("arm64-v8a", "armeabi-v7a", "x86", "x86_64")
            }
        }

        ndk {
            abiFilters += listOf("arm64-v8a", "armeabi-v7a", "x86", "x86_64")
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = true
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"))
        }
        debug {
            isDebuggable = true
            isJniDebuggable = true
        }
    }

    externalNativeBuild {
        cmake {
            path = file("CMakeLists.txt")
            version = "3.22.1"
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    kotlinOptions {
        jvmTarget = "17"
    }

    sourceSets {
        getByName("main") {
            assets.srcDirs("src/main/assets")
        }
    }

}

dependencies {
    implementation("androidx.core:core-ktx:1.12.0")
    implementation("androidx.appcompat:appcompat:1.6.1")
}

// Auto-copy APK to network share after build
tasks.register("copyApkToShare") {
    doLast {
        val apk = file("build/outputs/apk/debug/app-debug.apk")
        if (apk.exists()) {
            val dest = "\\\\ALENANDAMINA\\Photos\\MU\\app-debug.apk"
            apk.copyTo(file(dest), overwrite = true)
            println("APK copied to $dest")
        } else {
            println("APK not found at ${apk.absolutePath}, skipping copy")
        }
    }
}

tasks.whenTaskAdded {
    if (name == "assembleDebug") {
        finalizedBy("copyApkToShare")
    }
}
